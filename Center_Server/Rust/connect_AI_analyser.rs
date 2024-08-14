
use std::{net::{TcpStream}};
use std::thread;
use std::time::Duration;
//enum //private Server <-> Ai_Server 간 Enum값
use std::sync::{Mutex,Arc};

use crate::AGENT_MANAGEMENT_functions;

use crate::MYSQL::{self,DB_inst_METHODS};

use crate::Utils;
 // 파일 입출력

pub enum Server_AI_enum_{
    Static_analyse = 1, // 정적분석요청 Server -> AI
    Dynamic_analyse = 2, // 동적분석요청 Server -> AI
    Vt_analyse = 3, // VirusTotal 분석요청 Server -> AI

    Static_analyse_with_Vt = 4, // 정적분석 + Vt 분석
    Dynamic_analyse_with_Vt = 5,  // 동적분석 + Vt 분석

    ALL_analyse = 6, // 순서대로 정적->동적->Vt 를 한꺼번에 수행한다.

    Yes = 1029,  // 공용 ( 대부분은 AI의 응답용 )
    No = 1030  // 공용 ( 대부분은 AI의 응답용 )
}

pub fn Start_Connect_to_AI_analyser_Server(Mysql:Arc<Mutex<MYSQL::DB_inst>>, Vt_api:String){
    
    
    loop{
        let retry_time_for_sec = 3;
        let mut stream:TcpStream;
        println!("a");
        stream = match TcpStream::connect("127.0.0.1:3090"){ // AI서버에 연결
            Ok(result_stream)=>{println!("AI서버와 연결되었습니다.");result_stream}
            Err(_)=>{
                println!("AI 서버와 연결할 수 없습니다! {}초 후에 재시도합니다..",retry_time_for_sec);
                thread::sleep(Duration::new(retry_time_for_sec,0));
                continue;
            }
        };
        let mut stream= AGENT_MANAGEMENT_functions::TCP_STREAM_Instance::new(&mut stream, 999999,999999);
        
        loop{
            /*
                Server -> AI_Server 전송 유의사항

                서버에서 AI에게 요청할 때, 주기적으로 전송해야하는데, 
                
                동적/정적/Vt 이다.
                동적/정적의 경우는 그냥 보내고 AI의 응답만 처리하면 되지만, Vt의 경우 추가로 API 키를 요구하고 있다.

                {Enum 4 byte} + { length + Raw_data } + {"_END"}

             */

            /* 아래 반환하는 bool값은 Socket 통신의 상황임 */
            /* 아래 메서드 결과처리 법 -> None을 받으면 소켓을 닫도록 함 */
            if SEND_DATA_to_AI_andthen_Receive(&Mysql, & mut stream ,&Vt_api) == None{ // 이 함수안에서 AI에게 명령을 전달한다 / Mysql , TcpStream 관리객체, VirusTotal API값
                
                break;
            }
            
        }
        stream.try_Disconnect(); //연결종료 시도 
        drop(stream);
        continue
    }
    
    
    

    //SQL 질의 하고 나서 file 테이블을 지속적으로 조회해, AI서버에게 분석 "요청" 후 "수신" 하는 방식. ( 분석 결과는 NULL인 값에 쓰면 된다. )
}

fn SEND_DATA_to_AI_andthen_Receive( Mysql:&Arc<Mutex<MYSQL::DB_inst>>, stream:& mut AGENT_MANAGEMENT_functions::TCP_STREAM_Instance, Vt_api:&String )->Option<bool>{ // Mysql 를 조회하여 얻어온 수 만큼 AI에게 넘긴다
    /*
        <Server -> AI_Server 로 갈 때>
        
        1. MYsql 질의 -> Vec<Vec<String>>
        2. MYsql에 있는 경로를 통해 메모리에 임시 저장 -> Vec<u8>
    */

    // 질의하기. ( _vt_ 결과가 NULL 일 때만 가져온다)
    let FILE_info = Mysql.lock().unwrap().get_Query_from_MySql("SELECT File_SHA256, signature, size, saved_path FROM file WHERE ( static_ai_result_string IS NULL or static_ai_result_float32 IS NULL ) && (signature = 'exe')");
    if FILE_info.len() < 1{  
        //println!("AI_analyser 의 SEND_DATA_to_AI 함수에서 Mysql 'get_Query_from_MySql' 길이가 0임! ");
        println!("VT분석할게 없음;;ㅋ");
        thread::sleep(Duration::new(3,0));
        return Some(false);
    }
    println!("FILE_info -> {:?}", FILE_info);
    // 하나씩 가져오기
    for (index, DATA_from_DB) in FILE_info.iter().enumerate(){
        /*
            DATA_from_DB[0] -> 파일의 SHA256해시값
            DATA_from_DB[1] -> 파일의 시그니처 ( exe, pdf 등등 )
            DATA_from_DB[2] -> 파일의 사이즈 
            DATA_from_DB[3] -> 파일이 Server에 설치된  절대 경로

            Vt_api -> VirusTotal API값

            {ENUM - 4바이트 } + {Vt_API - 문자열} +  {파일 SHA256 - 64바이트} + {파일 시그니처 - 동적} + {파일 크기 - 4바이트} + {파일 바이너리 (개큼+동적)} + {_END}

         */


            /*
                실제 데이터 바이너리 가져오기 ( 성공적으로 가져와야지만, AI 요청가능.)
            */
        match Utils::Load_from_Disk(&DATA_from_DB[3].replace("\\",r"\\")){
            Some(Binary)=>{


                match TRY_Static_analyse_with_Vt_Function( // 정적 + VT분석 시도
                    stream,
                    Mysql,
                    &Vt_api.as_bytes().to_vec(),
                    &DATA_from_DB[0].as_bytes().to_vec(),
                    &DATA_from_DB[1].as_bytes().to_vec(),
                    &( (String_to_u32( &DATA_from_DB[2].to_string()).unwrap().to_le_bytes()).to_vec()),
                    &Binary,
                    &"_END".as_bytes().to_vec()
                ){
                    Some(bools)=>{
                        match bools{
                            true=>{
                                println!("TRY_Static_analyse_with_Vt_Function 성공");
                                continue
                            },
                            false=>{
                                println!("TRY_Static_analyse_with_Vt_Function 실패");
                                continue
                            }
                        }
                    },
                    None=>return None
                }






                

            },
            None=>{
                Utils::Remove_File_in_Disk(&DATA_from_DB[3].replace("\\",r"\\")); // 없으면 실제 파일 삭제
                Mysql.lock().unwrap().get_Query_from_MySql(format!("DELETE FROM file WHERE saved_path = '{}'", DATA_from_DB[3].replace("\\",r"\\") ).as_str()); // file 테이블에서도 삭제
                println!("AI를 위한 바이너리 조차 못가져옴!");
                continue;
            }
        };
        
        
    }


    Some(true)
}

fn TRY_Static_analyse_with_Vt_Function(AI_socket_manager:&mut AGENT_MANAGEMENT_functions::TCP_STREAM_Instance, Mysql:&Arc<Mutex<MYSQL::DB_inst>>, VT_API:&Vec<u8>, SHA256:&Vec<u8>, FILE_signature:&Vec<u8>, FILE_size:&Vec<u8>, FILE_binary:&Vec<u8>, END_str:&Vec<u8> )->Option<bool>
{

    // 데이터 하나로 합치기
    let mut SEND_DATA:Vec<u8> = Vec::new();
    SEND_DATA.extend(    (Server_AI_enum_::Static_analyse_with_Vt as u32).to_le_bytes()   );
    extend_data(&mut SEND_DATA, VT_API);
    extend_data(&mut SEND_DATA, &( (*SHA256).clone()) );
    extend_data(&mut SEND_DATA, FILE_signature);
    extend_data(&mut SEND_DATA, FILE_size);
    extend_data(&mut SEND_DATA, FILE_binary);
    SEND_DATA.extend(END_str);
    println!("Static_analyse_with_Vt 전송하겠습니다.->");
    match AI_socket_manager.SEND_DATA(SEND_DATA){ // [1/2]
        true=>{
            match AI_socket_manager.Receiving_from_Agent(){ //[2/2]
                Some(receive_data)=>{
                    
                    /* 잘 받았는지 확인 후 Parsing */

                    // 분석결과 성공인지 아닌지 확인.
                    if !is_Success_Receive(&receive_data) { return Some(false);}
                    
                    // 이제 Parsing
                    let Parsed_data = Utils::Get_RAW_DATA_until__END(&mut 0, &mut 4, &receive_data); // 파싱
                    /*
                        [데이터를 어떻게 얻어오는가?]
                        [0..4] yes or no 결과
                        ~
                        [길이4+분석결과y문자열] + [길이4+분석결과 수치 float] + ["_END"]

                     */
                    let Analysed_Y_value = String::from_utf8(  Parsed_data[0].clone() ).unwrap();

                    let mut tmp_for_float:[u8;4] = [0u8;4];
                    tmp_for_float.copy_from_slice( Parsed_data[1].as_slice() );
                    let ANalysed_Y_float = f32::from_le_bytes(tmp_for_float);
                    
                    println!("TRY_Static_analyse_with_Vt_Function 성공 결과 -> 분석결과:{}, float->{}",Analysed_Y_value,ANalysed_Y_float );

                    /* DB에 넣자! */
                    let query = format!(
                        "UPDATE file SET static_ai_result_string = '{}', static_ai_result_float32 = '{}' WHERE File_SHA256 = '{}'",
                        Analysed_Y_value,
                        ANalysed_Y_float.to_string(),
                        String::from_utf8( (*SHA256).clone() ).unwrap()
                    );

                    Mysql.lock().unwrap().get_Query_from_MySql(query.as_str());
                    Some(true)
                },
                None=>return None
            }
        },
        false=>return None
    }
    
    
}


fn is_Success_Receive(Response_data:&Vec<u8>)->bool{
    if Response_data[0..4] == (Server_AI_enum_::Yes as u32).to_le_bytes(){
        println!("AI서버의 분석결과는 성공입니다!");
        true
    }
    else if Response_data[0..4] == (Server_AI_enum_::No as u32).to_le_bytes() {
        println!("AI서버의 분석결과는 실패입니다!");
        thread::sleep(Duration::new(20,0));
        false
    }else {
        println!("AI서버의 분석결과를 알 수 없음!");
        thread::sleep(Duration::new(20,0));
        false
    }
}


fn Bytes_to_U32(bytes: &[u8])->u32{

    let mut sample = [0u8;4];
    sample.copy_from_slice(bytes);
    u32::from_le_bytes(sample)
}


fn extend_data(Raw_DATA:&mut Vec<u8>, input_data:&Vec<u8>){
    Raw_DATA.extend( (input_data.len() as u32).to_le_bytes() );
    Raw_DATA.extend( input_data.clone() );

}

fn String_to_u32(get_DATA: &String)->Option<u32>{
    match get_DATA.parse::<u32>(){
        Ok(n)=>{ Some(n) },
        Err(_)=>{None}
    }
}