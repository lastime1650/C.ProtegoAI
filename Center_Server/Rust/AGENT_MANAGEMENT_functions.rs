/*
    Agent_1vs1_Manager 의 기능을 세분화하기 위한 함수들의 집합소
*/
use std::net::{Shutdown, TcpStream};
use std::io::{Read, Write};
use std::io::BufReader;


use std::time::Duration;


/* Agent <-> Server 간 사용되는 ENUM */
pub enum SERVER_COMMAND{
    Err = 0,
    GET_file_bin = 1,
    GET_collected_data = 1650, // 윈도우 커널 병렬 스레드를 통해 저장된 전역변수를 가져오도록 함

    GET_action_process_creation = 100,
    GET_action_process_network = 101,
    GET_action_network = 200,

    Are_you_ALIVE = 999,

    Create = 10,
    Remove = 11,
    Inbound = 20,
    Outbound = 21,

    Yes = 1029,
    No = 1030
}


/* [개발됨] TcpStream을 총 관리하는 인스턴스를 만들고, 이 인스턴스를 Mutex하자 */
pub struct TCP_STREAM_Instance{
    Agent_TcpStream : TcpStream,
    Agent_Receive_Buf : BufReader<TcpStream>
    
}
impl TCP_STREAM_Instance{ // 메서드 총 4개 ( new생성자포함 )

    pub fn new(AGENT_TcpStream_parm: &mut TcpStream, Set_Read_Timeout_for_Secs:u64, Set_Write_Timeout_for_Secs:u64)->Self { // TcpStream은 인스턴스 생성 시, 그냥 TcpStream 주소만 넘겨라 ( 복제는 이 new 메서드안에서 다 해줌! )

        AGENT_TcpStream_parm.set_read_timeout(Some(Duration::new(Set_Read_Timeout_for_Secs,0))); // Tcp스트림 수신 타임아웃
        AGENT_TcpStream_parm.set_write_timeout(Some(Duration::new(Set_Write_Timeout_for_Secs,0))); // Tcp스트림 송신 타임아웃
        
        TCP_STREAM_Instance{
            Agent_TcpStream : AGENT_TcpStream_parm.try_clone().unwrap(),
            Agent_Receive_Buf : BufReader::new(  AGENT_TcpStream_parm.try_clone().unwrap() )
        }
    }

    /*  받기  */
    pub fn Receiving_from_Agent(&mut self)->Option<Vec<u8>>{
        let mut GET_BUFFER_SIZE = [0u8;4];
        match self.Agent_Receive_Buf.read(&mut GET_BUFFER_SIZE){ // 1차 리스닝 
            Ok(0)=>{println!("연결끊김1-1");None},
            Ok(Size)=>{
    
                if GET_BUFFER_SIZE.len() < 1 {println!("데이터를 얻었지만 길이가 0임! [1]");return None}
                //println!("초기 BUFFER _> {:?}", GET_BUFFER_SIZE);
                /* 성공적으로 데이터를 받았을 때 */
                let DATA_SIZE:u32 = u32::from_le_bytes(GET_BUFFER_SIZE);
                println!("성공 1 길이 -> {}", DATA_SIZE);
                let mut DATA_BUFFER = vec![0u8;DATA_SIZE as usize]; 
                
                
                let mut trace_DATA_SIZE = 0;
                let mut ALL_of_DATA:Vec<u8> = Vec::new();
                loop{
                    match self.Agent_Receive_Buf.read(&mut DATA_BUFFER){ // 2차 리스닝 ( 실제 데이터를 "DATA_BUFFER" 변수안에 저장하여 가져옴 )
                        Ok(0)=>{println!("연결끊김2-1");return None},
                        Ok(RAW_DATA)=>{
                            trace_DATA_SIZE += RAW_DATA;
                            //println!("마지막 두번째 BUFFER _> {:?}", DATA_BUFFER);
                            if DATA_BUFFER.len() >= 4 { // 4이상일 때,,, SUCCESS
                                /* 목표성공  하지만, AGENT에서 너무 큰 값을 보내거나, 나눠서 보내는 경우가 분명히 있어서, loop{} 완성된 데이터를 꺼내서 정상적으로 가져와야한다.*/
                                println!("데이터를 성공적으로 얻은 것 같습니다! -> 최종 리시브 길이 ->> [ {} / {} ]\n",RAW_DATA, DATA_SIZE); 
                                if trace_DATA_SIZE as usize != DATA_SIZE as usize{
                                    /* 반복적으로 가져와야함 */
                                    let need_more_buffer_size = DATA_SIZE as usize - RAW_DATA as usize;
                                    println!(" 아직 데이터를 모두 받은게 아닙니다. 남은 사이즈 -> {}", need_more_buffer_size);
                                    
                                    ALL_of_DATA.extend( DATA_BUFFER[0..RAW_DATA].to_vec() );

                                    //DATA_SIZE = need_more_buffer_size as u32; // 받아올 길이 갱신 원래보다 값이 작아야한다
                                    DATA_BUFFER = vec![0u8; need_more_buffer_size]; // 새로 받아올 그릇 , 원래보다 길이가 작아야한다
                                    continue;
                                }else{
                                    ALL_of_DATA.extend( DATA_BUFFER[0..RAW_DATA].to_vec() );
                                    return Some(ALL_of_DATA) // 최종적으로 값을 리턴 ------------------> Vector return
                                }
                                
                                
                            }
                            else{
                                /* 아예 실패작 */
                                println!("이도저도 아닌 실패작![3]");
                                return None
                            }
        
        
                            
                        },
                        Err(_)=>{println!("연결끊김2-2");return None}
        
                    }
                }
                /* [2/2] 그 길이 만큼 동적할당하여 데이터 얻기 */
                
    
            },
            Err(_)=>{println!("연결끊김1-2");None}
        }
    }

    /*  보내고 , 받기  */
    pub fn SEND_DATA_with_SERVER_COMMAND(&mut self, send_data: Vec<u8>, server_command_type: SERVER_COMMAND)->Option<Vec<u8>>{

        let command_type_num:u32 = server_command_type as u32;
        println!("명령을 내리겠습니다..->{}", command_type_num);
        /*    서버명령(4b) + 보낼 데이터(가변) 을 전송한다.    */
        let server_command_byte = command_type_num.to_le_bytes();
        let mut server_command_byte = server_command_byte.to_vec();
        server_command_byte.extend( send_data );
        // 소유권이 server_command_byte에게 가버림
    
        if self.SEND_DATA(server_command_byte) == false{ // 전송하고,
            None
        }else{
            match self.Receiving_from_Agent(){
                Some(d)=>{
                    Some(d)
                   
                },
                None=>{
                    return None}
            }
        }
    
       
        
    }

    /*  보내기  */
    pub fn SEND_DATA(&mut self, send_data: Vec<u8> ) -> bool{



        //TCP_STREAM.set_write_timeout(Some(Duration::new(5,0))); // TCP전달 시, 타임아웃 설정
    
    
        /*
            길이 ( 4바이트 ) 먼저 보내기 [1/2]
         */
        let send_data_length1:u32 = send_data.len() as u32;
        let send_data_length = send_data_length1.to_le_bytes();
        match self.Agent_TcpStream.write_all(&send_data_length){
            Ok(_)=>{//println!("SEND_DATA Ok(길이전송) -> [1/2] 길이 전송 완료");
       
                let test = send_data.as_slice(); // vector를 배열로
                match self.Agent_TcpStream.write_all( test ){
                    Ok(_)=>{
                        //println!("SEND_DATA Ok(길이전송) -> [2/2] 실제 데이터 전송 완료"); 
                        true
                    },
                    Err(e)=>{println!("SEND_DATA -> [2/2] 전송중에서 5초 타임아웃됨");false},
                }
    
             },
            Err(e)=>{println!("SEND_DATA -> [1/2] 전송중에서 5초 타임아웃됨");false},
        }
    
    
    }

    // 연결종료를 시도한다
    pub fn try_Disconnect(&mut self)->bool{
        match self.Agent_TcpStream.shutdown(Shutdown::Both){
            Ok(n)=>{true},
            Err(_)=>{false}
        }
    }

}

/* 
    데이터를 성공적으로 얻으면 Some() 안에 값이 있고,
    없으면 None이 되어버림!

pub fn Receiving_from_Agent(Buf: &mut BufReader<TcpStream>)->Option<Vec<u8>>{
    let mut GET_BUFFER_SIZE = [0u8;4];
    match Buf.read(&mut GET_BUFFER_SIZE){ // 1차 리스닝 
        Ok(0)=>{println!("연결끊김1-1");None},
        Ok(Size)=>{

            if(GET_BUFFER_SIZE.len() < 1){println!("데이터를 얻었지만 길이가 0임! [1]");return None}
            //println!("초기 BUFFER _> {:?}", GET_BUFFER_SIZE);
            성공적으로 데이터를 받았을 때 
            let DATA_SIZE:u32 = u32::from_le_bytes(GET_BUFFER_SIZE);
            println!("성공 1 길이 -> {}", DATA_SIZE);
            let mut DATA_BUFFER = vec![0u8;DATA_SIZE as usize]; 
            println!("성공 2 길이 -> {}",DATA_BUFFER.len());



             [2/2] 그 길이 만큼 동적할당하여 데이터 얻기 
            match Buf.read(&mut DATA_BUFFER){ // 2차 리스닝 ( 실제 데이터를 "DATA_BUFFER" 변수안에 저장하여 가져옴 )
                Ok(0)=>{println!("연결끊김2-1");None},
                Ok(RAW_DATA)=>{
                    //println!("마지막 두번째 BUFFER _> {:?}", DATA_BUFFER);
                    if(DATA_BUFFER.len() >= 4){ // 4이상일 때,,, SUCCESS
                        /* 목표성공 */
                        println!("데이터를 성공적으로 얻은 것 같습니다!\n=>{:?}", DATA_BUFFER); 
                        Some(DATA_BUFFER) // 최종적으로 값을 리턴 ------------------> Vector return
                    }
                    else if(DATA_BUFFER.len() == 6){ // 6이건 테스트용
                        /* 커널 소켓 통신 테스트용 */
                        println!("커널 소켓 통신 테스트용 입니다.");
                        Some(DATA_BUFFER) // 최종적으로 값을 리턴 ------------------> Vector return
                    }
                    else if(DATA_BUFFER.len() < 1){
                        println!("데이터를 얻었지만 길이가 0임! [2]");
                        return None 
                    }
                    else{
                        /* 아예 실패작 */
                        println!("이도저도 아닌 실패작![3]");
                        return None
                    }


                    
                },
                Err(_)=>{println!("연결끊김2-2");None}

            }

        },
        Err(_)=>{println!("연결끊김1-2");None}
    }
}

/* 전송 + Enum값 */

pub fn SEND_DATA_with_SERVER_COMMAND(TCP_STREAM: &mut TcpStream , Buf: &mut BufReader<TcpStream>, send_data: Vec<u8>, server_command_type: SERVER_COMMAND)->Option<Vec<u8>>{
    
    /*    서버명령(4b) + 보낼 데이터(가변) 을 전송한다.    */
    let mut server_command_byte = (server_command_type as u32).to_le_bytes();
    let mut server_command_byte = server_command_byte.to_vec();
    server_command_byte.extend( send_data );
    // 소유권이 server_command_byte에게 가버림

    SEND_DATA(TCP_STREAM, server_command_byte);// 보내고 

    match Receiving_from_Agent(Buf){//바로 받아야함

        Some(data)=>{Some(data)},
        None=>{ None}

    }


}


/* 전송용 */
pub fn SEND_DATA(TCP_STREAM: &mut TcpStream , send_data: Vec<u8> ) -> Result<(),()>{



    //TCP_STREAM.set_write_timeout(Some(Duration::new(5,0))); // TCP전달 시, 타임아웃 설정


    /*
        길이 ( 4바이트 ) 먼저 보내기 [1/2]
     */
    let mut send_data_length1:u32 = send_data.len() as u32;
    let send_data_length = send_data_length1.to_le_bytes();
    match TCP_STREAM.write_all(&send_data_length){
        Ok(_)=>{println!("SEND_DATA Ok(길이전송) -> [1/2] 길이 전송 완료");
   
            let test = send_data.as_slice(); // vector를 배열로
            match TCP_STREAM.write_all( test ){
                Ok(_)=>{println!("SEND_DATA Ok(길이전송) -> [2/2] 실제 데이터 전송 완료"); Ok(())},
                Err(e)=>{println!("SEND_DATA -> [2/2] 전송중에서 5초 타임아웃됨");Err(())},
            }

         },
        Err(e)=>{println!("SEND_DATA -> [1/2] 전송중에서 5초 타임아웃됨");Err(())},
    }


}

*/