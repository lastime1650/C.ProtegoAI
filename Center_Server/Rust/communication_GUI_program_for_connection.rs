/*

    파이썬 GUI 프로그램과 상호작용한다.
*/
use std::thread::{self};
use std::net::{TcpListener, TcpStream};

use std::sync::{Mutex,Arc};

use crate::{Hashing, Utils};
use crate::MYSQL::{self};
use crate::MYSQL::DB_inst_METHODS;
use crate::AGENT_MANAGEMENT_functions::{TCP_STREAM_Instance,SERVER_COMMAND};
use crate::Share_TCP_and_AGENT_ID;

pub fn Start_Listen(IP_info_: String, Mysql_sharing_instance: Arc<Mutex<MYSQL::DB_inst>>, Share_Agent_ID__and__TCP_stream:Arc<Mutex<Share_TCP_and_AGENT_ID::AGENT_GUI_share>>){
     /* TCP서버 준비 작업.*/
     let TCP_SERVER_Listener = TcpListener::bind(IP_info_).unwrap();
    
     /* TCP 리스닝 상태 전환 */
     for (index, STREAM) in TCP_SERVER_Listener.incoming().enumerate(){
         match STREAM{
             Ok(stream)=>{
                 println!(" {}에이전트와 연결되었습니다. ", (stream.peer_addr().unwrap()) );
                 
                 let CLONED = Arc::clone(&Mysql_sharing_instance);//각 스레드당 공유자원 전달를 위해 복제
                 let Share_CLOEND = Arc::clone(&Share_Agent_ID__and__TCP_stream);
                 thread::spawn(move || {
                     /* 연결된 에이전트를 1:1 관리위하여 스레드 생성 */
                     GUI_program_manager(
                        stream.try_clone().unwrap(), // [0] 에이전트 TCP 객체
                        CLONED,
                        Share_CLOEND); //[1] MYSQL 공유자원 ( 연결 된 상태 *연결 안되면 안댐!!)
                 });
                 
             },
             Err(_)=>{
                 println!("해당 에이전트와는 연결할 수 없습니다.");
             },
         }
     }
}

pub enum GUI_enums{
    no = 0,
    Get_ALL_Agents = 1, // 현재 DB에 올려진 모든 Agent의 데이터를 가져온다 (  )
    Get_LIVE_Agents = 2, // 현재 Server와 연결 중인 Agent의 데이터만 가져온다 ( Server와 연결중이므로, 실시간 명령요청이 가능하다 

    Get_process_routine_TABLE=2000, // process_routine 테이블의 값을 가져와라!

    Send_Command_Process = 100, // GUI -> 프로세스 명령 전달
    Send_Command_Network = 101, // GUI -> 네트워크 명령 전달

    Get_LIVE_RUST = 1000, // GUI가 RUST와 지속적으로 통신중인지? 
    

    Create = 10,
    Remove = 11,
    Inbound = 20,
    Outbound = 21,

    Yes = 1029,
    No = 1030
}

pub fn GUI_program_manager(mut TCP_Stream:TcpStream, Mysql_sharing_instance: Arc<Mutex<MYSQL::DB_inst>>, Share_Agent_ID__and__TCP_stream:Arc<Mutex<Share_TCP_and_AGENT_ID::AGENT_GUI_share>>){
    
    
    
    println!(" {}에이전트 1:1 관계 성립됨! ", (TCP_Stream.peer_addr().unwrap()) );
    let mut GUI_TcpStream_Instance = TCP_STREAM_Instance::new(&mut TCP_Stream, 99999, 99999); // BufReader는 내부에서 만들어짐

    /* GUI_ID는 GUI프로그램에서 얻어야한다.  */
    /* GUI 프로그램의 고유한 ID 얻기 */
    
    let mut GUI_ID = [0u8;128];
    match GUI_TcpStream_Instance.Receiving_from_Agent(){
        Some(DATA)=>{
            if DATA.len() == 128 {
                GUI_ID.copy_from_slice(&DATA[0..128]);
            }else{
                println!("최초 SHA512 얻기 실패!");
                return;
            }
            
        },
        None => {println!("최종데이터 실패!");return}
    }

    /* GUI로부터 SHA512얻으면 연결성공을 connecting_guis DB 에 저장 (수정필요)*/
    if Mysql_sharing_instance.lock().unwrap().Query_connecting_gui(&(std::str::from_utf8(&GUI_ID).unwrap().to_string()), false) != true {
        println!("GUI_ID 삽입 실패");
        return;
    }

    
    
    loop{
        /* 무한 리시브 */
        /*  동기 처리임. GUI가 명령을 전달하면, GUI는 수신대기 상태임  */
        match GUI_TcpStream_Instance.Receiving_from_Agent(){
            Some(DATA)=>{
                println!("최종데이터 -> {:?}", DATA);
                match GUI_Parsing_data( &GUI_ID, &DATA, &Mysql_sharing_instance, &Share_Agent_ID__and__TCP_stream){
                    Some(Success_Parsed)=>{
                        GUI_TcpStream_Instance.SEND_DATA(Success_Parsed);
                    },
                    None=>{ println!("파싱 실패");continue}
                }
            },
            None => {println!("최종데이터 실패!");return}
        }

    }
    /*
        [노트]
            GUI TCP 스트림도 Instance 화 하여 사용하도록 하였다.
     */
    
}


/* GUI의 명령을 해석하는 함수 */
pub fn GUI_Parsing_data( GUI_ID_for_compare: &[u8], DATA:&Vec<u8>, Mysql_sharing_instance: &Arc<Mutex<MYSQL::DB_inst>>,Share_Agent_ID__and__TCP_stream:&Arc<Mutex<Share_TCP_and_AGENT_ID::AGENT_GUI_share>>) -> Option< Vec<u8> >{
    /*
    
        <GUI가 보내는 데이터의 구조>
        {SHA512/128바이트} + {명령/4바이트} + {  { 길이/4바이트 } + {데이터/ "가변" 바이트}  } < - 이 2 세트는 있을 수 있고 아예 없을 수 이씅ㅁ 

        명령은 하나만 받아서 처리한다. 
     */
    let mut start_index = 0 as usize;
    let mut last_index = 128 as usize;

    //let mut GUI_ID = [0u8;128]; ///////////////////////////////////////////
    //GUI_ID.copy_from_slice(&DATA[start_index..last_index]); //{SHA512/128바이트} 추출


    let GUI_ID = match Get_GUI_ID(DATA,start_index,last_index){
        Some(parsed)=>{
            if GUI_ID_for_compare != parsed {
                println!("GUI 값이 달라서 파싱할 수 없습니다!>.");
                return None;
            }

            parsed // GUI_ID에 저장
        },

        None=>{
            return None;
        }
    };

    
    println!("GUI 값이 같습니다!");
    println!("이제 명령을 해석합니다. ");
    start_index = last_index;
    last_index = start_index + 4;
    let mut GUI_cmd= [0u8;4];//////////////////////////////////////////////
    GUI_cmd.copy_from_slice(&DATA[start_index..last_index]);
    let GUI_cmd = u32::from_le_bytes(GUI_cmd); // {명령/4바이트} 추출
    println!("명령값 (정수)-> {}", GUI_cmd);
    
    /* GUI의 명령을 처리한다.
         GUI -> SERVER ( Parsing )
        {GUI_ID} + {4bytes enum} + {{4byte}+{Raw_DATA(AGENT_ID)} - 길이기반}

     */

     let mut SEND_DATA_to_GUI:Vec<u8> = Vec::new(); // GUI에게 보낼 변수를 마련한다.


    /* 반환정보 -> { [4byte]+[AGENT_ID] } + {_END} */
    /* DB 질의 */
    if GUI_enums::Get_ALL_Agents as usize == GUI_cmd as usize { // 첫시작 ( GUI의 DashBoard.py에 버튼을 동적으로 생성하게 해줌 )
        println!("Get_ALL_Agents 를 얻었습니다.");
        /*  인자에 보낸 str 쿼리의 결과의 칼럼 수를 잘 알고 있어야한다.... Vec<String>으로 데이터를 보내야하므로, 동적임 */
        let Query_Result = Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql("SELECT AGENT_ID, timestamp from agent_list");
        for (index, data) in Query_Result.iter().enumerate(){
            
            APPEND_DATA__into__send_data(&mut SEND_DATA_to_GUI, data[0].as_bytes() );
        }
    }
    /* 반환정보 -> { 4byte - ENUM } + {_END}  - 8바이트 */
    /* 직접 AGENT 요청 */
    else if GUI_enums::Get_LIVE_Agents as usize == GUI_cmd as usize { // Agent가 현재 살아있는가? ( GUI -> Server(여기) -> Agent ) GUI 권한으로 확인한다!
        /* {GUI_ID} + {4b} + ( {길이4b} + {Raw_Data (AGENT_ID)} )  */
        //println!("Get_LIVE_Agents 를 파싱합니다. Data -> {:?}", DATA);
        let parsed = Get_RAW_DATA_until__END(&mut start_index, &mut last_index, DATA ); //2차원 배열을 반복적으로 리턴한다
        
        let AGENT_ID = ( std::str::from_utf8( parsed[0].as_slice() ).unwrap()).to_string(); // AGENT_ID
        //println!("AGENT_ID --> {}", AGENT_ID);


        /* 1차공유변수를 match하지 않고 변수에 저장해서 스레드에서 알아서 처리하면 성능향상 (점유요청 대기시간을 최대한 줄여라!!) */
        let Get_Agent_TcpStream_DATA: Option<Arc<Mutex<TCP_STREAM_Instance>>> = Share_Agent_ID__and__TCP_stream.lock().unwrap().Get_TCP_Stream_from_Agent_ID(&AGENT_ID); // 1차공유변수를 match하지마라. ( 공유객체변수를 AGENT_ID로 필터링하여 " TcpStream 관리 객체 " 찾기 )
        match Get_Agent_TcpStream_DATA{
            Some(data)=>{
                SEND_DATA_to_GUI.extend((GUI_enums::Yes as u32).to_le_bytes() );
                /*
                /* TcpStream 관리 객체 찾았을 때 */
                /* Agent TCP스트림을 점유해서 사용할 땐, Lock한 상태에서 사용하라. */
                match data.lock().unwrap().SEND_DATA_with_SERVER_COMMAND(AGENT_ID.as_bytes().to_vec(), SERVER_COMMAND::Are_you_ALIVE){ // 2차
                    Some(result) => {
                        println!("Agent의 응답 -> {:?}", result);

                        

                        /*
                            여기서 result 값은 ( Yes 또는 No ) ENUM 값 -> {리틀 엔디언 4바이트}  이어야 하며 무조건 길이가 4이어야만 함
                         */
                        let mut result_to_u32 = [0u8;4];
                        result_to_u32.copy_from_slice(&result.clone());
                        if result.len() == 4 && result_to_u32 == (GUI_enums::Yes as u32).to_le_bytes() {
                            SEND_DATA_to_GUI.extend((GUI_enums::Yes as u32).to_le_bytes() );// ENUM값 반환 -> Yes
                        }else{
                            println!("길이가 4가 아님; ");
                            SEND_DATA_to_GUI.extend((GUI_enums::No as u32).to_le_bytes());// ENUM값 반환 -> No
                        }
                    },
                    None=>{
                        println!("Agent의 응답이 없음!");
                        SEND_DATA_to_GUI.extend((GUI_enums::No as u32).to_le_bytes());// ENUM값 반환 -> No
                    }
                }
                 */

            },
            None=>{
                println!("{} <-Agent는 현재 연결되고 있지 않음!",AGENT_ID);
                SEND_DATA_to_GUI.extend((GUI_enums::No as u32).to_le_bytes());
            }
        }
       
        

    }
    else if GUI_enums::Get_LIVE_RUST as u32 == GUI_cmd as u32 {
        // GUI ---> RUST rust 살아있니?
        SEND_DATA_to_GUI.extend( (GUI_enums::Yes as u32).to_le_bytes() );
        println!("Get_LIVE_RUST 받음!");
    }
    else if GUI_enums::Send_Command_Process as u32 == GUI_cmd as u32 {
        // GUI -> Rust -> Agent에게 명령을 전송할 데이터 처리
        let parsed_data = Utils::Get_RAW_DATA_until__END(&mut start_index, &mut last_index, DATA);

        /*
            parsed_data[0] -> Agent_ID
            parsed_data[1] -> Block or Permit.. 문자열 바이트
            parsed_data[2] -> SHA256
            parsed_data[3] -> Process or Network
        
         */
        
        let AGENT_ID:String = String::from_utf8(parsed_data[0].clone()).unwrap();
        let Method:String = String::from_utf8(parsed_data[1].clone()).unwrap();
        let Sha256:String = String::from_utf8(parsed_data[2].clone()).unwrap();
        let Type:String = String::from_utf8(parsed_data[3].clone()).unwrap();
        println!("proecss 결과 -> {} {} {}",Method,Sha256,Type );
        
        

        // Agent가져오기
        let Agent_Manager = Share_Agent_ID__and__TCP_stream.lock().unwrap().Get_TCP_Stream_from_Agent_ID(&AGENT_ID);
        match Agent_Manager{
            Some(agent_manager)=>{
                /* 에이전트 직접 소켓 통신 객체 가져옴 */
                
                /*
                    1. {Method}
                    2. {SHA256}
                    3. {Type} 
                    $ Create, Remove, Inbound, Outbound는 TYPE에 따라 결정되며, 커널에서 처리할 것임.

                    ! 단 Process_Routine + Remove + BLOCK 은 처리할 수 없다. ( 프로세스 제거를 하지 않도록 BLOCK 한다는 의미.)
                */
                let mut send_data: Vec<u8>= Vec::new();
                Utils::APPEND_DATA__into__send_data(&mut send_data, Method.as_bytes());
                Utils::APPEND_DATA__into__send_data(&mut send_data, Sha256.as_bytes());
                Utils::APPEND_DATA__into__send_data(&mut send_data, Type.as_bytes());
                

                send_data.extend("_END".as_bytes());

                match agent_manager.lock().unwrap().SEND_DATA_with_SERVER_COMMAND(send_data, SERVER_COMMAND::GET_action_process_creation){
                    Some(receive_agent_data)=>{
                        /* Agent로부터 "성공"응답을 받았을 때, */
                        SEND_DATA_to_GUI.extend((GUI_enums::Yes as u32).to_le_bytes());
                        /* mysql에 저장 */
                        Agent_Action_logging_db(
                            &AGENT_ID,
                            &Sha256,
                            &Method,
                            &format!("Process_{}",Type),
                            &Mysql_sharing_instance
                        );
                    },
                    None=>{
                        /* Agent로부터 "실패"응답 또는 부재응답 받았을 때, */
                        SEND_DATA_to_GUI.extend((GUI_enums::No as u32).to_le_bytes());
                    }
                }


            },
            None=>{
                SEND_DATA_to_GUI.extend((GUI_enums::No as u32).to_le_bytes());
            }
        }


    }
    else if GUI_enums::Get_process_routine_TABLE as u32 == GUI_cmd as u32 {
        let Raw_Data = Utils::Get_RAW_DATA_until__END(&mut start_index, &mut last_index, DATA);
        /*
            AGENT_ID 한개 얻어옴 
         */
        let AGENT_ID:String = String::from_utf8(Raw_Data[0].clone()).unwrap();
        let process_routine_TABLE_vec = Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(format!(" SELECT is_create, pid, full_path, exe_size, SHA256, DB_timestamp FROM process_routine WHERE AGENT_ID = '{}' ",AGENT_ID).as_str());
        
        for query_data_1 in process_routine_TABLE_vec.iter(){
            
            for query_data_2 in query_data_1.iter(){


                Utils::APPEND_DATA__into__send_data(&mut SEND_DATA_to_GUI, query_data_2.as_bytes() );
            }
        }
    }
    else{
        println!("GUI_enums 값에 속하지 않습니다. 오류!");
        return None;
    }


    SEND_DATA_to_GUI.extend("_END".as_bytes()); // "_END 로 마무리"
    return Some( SEND_DATA_to_GUI )

}


/* GUI에게 보낼 데이터 변수에 데이터를 {길이}+{실제데이터} 형태(길이-기반)로 넣어주는 함수 */
fn APPEND_DATA__into__send_data( SEND_DATA:&mut Vec<u8> , data: &[u8] ){

    let data_len_BYTEs = u32::to_le_bytes(data.len() as u32);
    SEND_DATA.extend(data_len_BYTEs.clone());

    SEND_DATA.extend(data);

    return;

}

/*
    GUI로부터 가져온 길이기반 데이터를 자동화로 Vec<[u8]>에 담아서 처리해준다.

    함수 호출 -> {GUI_ID} + { {4b}+{Raw_Data}  } + {"_END"} -> 끝 

*/
fn Get_RAW_DATA_until__END( start_index:&mut usize, last_index:&mut usize, Parsing_Data:&Vec<u8> )->Vec<Vec<u8>>{
    let mut Output_Vector:Vec< Vec<u8> > = Vec::new();
    loop{
        // Raw_DATA의 길이 가져오기
        *start_index = *last_index;
        *last_index = *start_index + 4;
        let mut Raw_DATA_len:[u8;4] = [0u8;4];
        Raw_DATA_len.copy_from_slice(&Parsing_Data[*start_index..*last_index]);
        if Raw_DATA_len == [95, 69, 78, 68] {
            /* _END 인 경우 True */
            break
        }
        let Raw_DATA_len = u32::from_le_bytes(Raw_DATA_len);
        //println!("길이 -> {}", Raw_DATA_len);

        // Raw_DATA 가져오기
        let mut tmp_Raw_DATA:Vec<u8> = Vec::new();
        *start_index = *last_index;
        *last_index = *start_index + Raw_DATA_len as usize;
        tmp_Raw_DATA.extend(&Parsing_Data[*start_index..*last_index]);
        //println!("Raw_DATA -> {:?}", tmp_Raw_DATA);

        Output_Vector.push(tmp_Raw_DATA.clone()); // 전역변수에 축적! 

    }
    return Output_Vector;
}

fn Get_GUI_ID( Parsing_Data: &Vec<u8>,start_index:usize,last_index:usize)->Option< [u8;128] > {
    let mut GUI_ID:[u8;128] =[0u8;128];

    if Parsing_Data[start_index..last_index].len() as usize != 128 as usize{
        return None;
    }

    GUI_ID.copy_from_slice(&Parsing_Data[start_index..last_index] );
    

    Some( GUI_ID )
}

fn Agent_Action_logging_db(
    AGENT_ID:&String, 
    SHA_256:&String,  
    Method:&String,
    Action_Type:&String,
    
    Mysql_sharing_instance: &Arc<Mutex<MYSQL::DB_inst>> ){

        Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(
            format!("INSERT INTO agent_action_list (AGENT_ID, Method, SHA256, Action_Type, timestamp, is_add_action) VALUES ('{}','{}','{}','{}','{}','{}') ",
        AGENT_ID,Method,SHA_256,Action_Type,Hashing::Get_Current_Timestamp_until_Micro().unwrap().0,"asd").as_str()
        );
    return;
    }