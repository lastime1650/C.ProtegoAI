use std::thread::{self};
use std::net::{TcpStream};

 // 소켓 송수신할 때 Timeout 설정하기 위한 객체

use std::sync::{Mutex,Arc};

use crate::MYSQL;
use crate::MYSQL::DB_inst_METHODS;
use crate::Hashing;

use crate::AGENT_MANAGEMENT_functions::{TCP_STREAM_Instance, SERVER_COMMAND};
use crate::Share_TCP_and_AGENT_ID;

use crate::Utils;

/* Agent가  GET_collected_data 명령의 응답으로 TYPE을 식별하는데 ,,, 사용되는 것*/
pub enum TYPE{
    process_routine = 1,
    network_filter = 10,

    API_HOOKED_MON = 100 // 에이전트에서 보내는 API 후킹 정보

}


/* Listening.rs -> here */

/* Agent의 TCP 스트림을 받아서 개별관리하도록 함 
    
    Start_Agent 함수는 1:1 에이전트를 관리하는 메인함수!
*/
pub fn Start_Agent(mut STREAM: TcpStream, Mysql_sharing_instance: Arc<Mutex<MYSQL::DB_inst>>, Share_Agent_ID__and__TCP_stream:Arc<Mutex<Share_TCP_and_AGENT_ID::AGENT_GUI_share>>){
    /*

        Agent와 1:1 간 통신 개별 스레드

     */
     /*
    
        초기에는 세션 데이터를 일단 전달한다.
        [SHA512 ( 128byte )] + [...]
        <<Agent_ID 생성>>

     */
    let get_port = STREAM.peer_addr().unwrap().port();
    let mut current_timestamp_for_AGENT_ID = Hashing::Get_current_timestamp_and_SHA512(get_port as u32).unwrap();

    let Agent_TcpStream_Inst = TCP_STREAM_Instance::new(&mut STREAM, 15, 20); // 15초
    let Agent_TcpStream_Inst_Mutex = Mutex::new(Agent_TcpStream_Inst);
    let mut Agent_TcpStream_Inst_Mutex__with__Arc: Arc<Mutex<TCP_STREAM_Instance>> = Arc::new( Agent_TcpStream_Inst_Mutex ); // Agent Tcp 스트림을 사용할 수 있는 객체

    
    
    /*
    
        *******
        자 그럼 Server가 Agent랑 통신하려면???????

        Agent_TcpStream_Inst_Mutex__with__Arc . lock() . unwrap() . {method 호출}

        *******
    
    
     */

    /* Agent ID 가 저장된 공간을 지정. */
    let Agent_ID_full_path = "\\??\\C:\\_Agent".to_string();
    let Agent_ID_full_path = Agent_ID_full_path.as_bytes();

    /* 생성한 SHA512(ascii)  + AgentID 절대경로(ascii) */
    current_timestamp_for_AGENT_ID.0.extend_from_slice(Agent_ID_full_path); // 기존 바이트에서 추가한다! 
    println!("{:?}", std::str::from_utf8(current_timestamp_for_AGENT_ID.0.as_slice()).unwrap());
    if Agent_TcpStream_Inst_Mutex__with__Arc.lock().unwrap().SEND_DATA( current_timestamp_for_AGENT_ID.0){ //전송 - AGENT_ID 교류 ( AGENT_ID 최초 전송 )


        /* 마지막 관문..... Agent의 AGENT_ID를 얻으면 성공이다 */
        let AGENT_ID__from__Agent = Agent_TcpStream_Inst_Mutex__with__Arc.lock().unwrap().Receiving_from_Agent(); // 받기 - AGENT_ID 교류 마지막 ( Agent로부터 AGENT_ID 받기 )
        match AGENT_ID__from__Agent{
            Some(AGENT_ID)=>{
                let AGENT_ID_bytes = AGENT_ID.clone();
                let AGENT_ID_string = std::str::from_utf8(AGENT_ID.as_slice()).unwrap().to_string();
                if AGENT_ID.len() == 128 && AGENT_ID_string == current_timestamp_for_AGENT_ID.1 {// Agent에서 가져온 AGENT_ID가 같은지 최종 점검 ( 이거 DB조회로 바꿔야댐 과거전적이 있는지 봐야하니까 )
                    println!("Agent 와의 AGENT_ID 교류 전송 성공!");


                    /* GUI 스레드에서 접근하도록, Agent_ID 공유자원을 1차 공유변수에 새로저장한다. */
                    Share_Agent_ID__and__TCP_stream.lock().unwrap().Insert_Agent_Data(&AGENT_ID_string, &mut Agent_TcpStream_Inst_Mutex__with__Arc); // TCP스트림 추가


                    /* DB에 해당 AGENT_ID가 고유하게 저장되어 있는지 없는 지 확인( 없으면 Insert_into함 ) *////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    Query_agent_list_Table(&AGENT_ID_string, &Mysql_sharing_instance);

                    /* 이제 connecting_agents 테이블에 현재 연결 중이라고 INSERT 하자 (폐지)*/
                    //Mysql_sharing_instance.lock().unwrap().set_Query_connecting_agent(&AGENT_ID_string, false); // 연결성공 DB에 현재 연결 중임을 저장



                    /* 최종성공 */
                    let mut thread_sleep_secs_time = 3;
                    loop{
                        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        /* 
                            Server <-> Agent 무한반복 통신

                            [*] 여기서는 DB 데이터를 축적하기 위해 무한 반복하며,
                            [**]중간에 GUI 스레드에서 Agent의 TcpStream을 잠시 점유할 수 있다. 

                            [Point!] 실제로 Agent와 통신하는 TCP스트림은 match로 길게 점유하도록 하자.
                                    -> 여기서 연결이 Agent와 끊기면, 1차 공유변수에서 먼저 삭제한 다음, GUI에서 이를 감지할 수 있기 때문이다.
                                    -> 단, Parsing의 경우 작업이 오래 걸리므로 스레드를 추가로 실행할 수도 있음; 
                         */
                        //SERVER_COMMAND::GET_collected_data 명령은, Agent 커널 자체적으로 병렬스레드로 저장한 전역변수의 데이터를 달라는 의미.
                        println!("1111111111111111111111111");
                        {
                            let mut result_data = Agent_TcpStream_Inst_Mutex__with__Arc.lock().unwrap();//.SEND_DATA_with_SERVER_COMMAND(AGENT_ID_bytes.clone(), SERVER_COMMAND::GET_collected_data);



                            //match Agent_TcpStream_Inst_Mutex__with__Arc.lock().unwrap().SEND_DATA_with_SERVER_COMMAND(AGENT_ID_bytes.clone(), SERVER_COMMAND::GET_collected_data){
                            match result_data.SEND_DATA_with_SERVER_COMMAND(AGENT_ID_bytes.clone(), SERVER_COMMAND::GET_collected_data){
                                Some(data)=>{
                                    
                                    println!("[AGENT 1:1 ] 무한루프 서버로부터 응답옴!");
                                    if data.len() == 4{
                                        /*
                                            길이가 4이고, 데이터가 아예 없으면 AGNET는 No를 보낸다.
                                        */
                                        if Utils::Bytes_to_U32(data.as_slice()) as u32  == SERVER_COMMAND::No as u32 {
                                            println!("[AGENT 1:1 ] 이런! No를 받음");
                                            thread_sleep_secs_time = 5; // 5초 대기
                                        }
                                    }else{
                                        /*
                                        Parsing은 비동기처리
                                        */
                                        let Mysql_sharing_instance__CLONED = Arc::clone(&Mysql_sharing_instance);
                                        let Agent_TcpStream_Inst_Mutex__with__Arc__CLONED = Arc::clone(&Agent_TcpStream_Inst_Mutex__with__Arc);
                                        thread::spawn( move || { 
                                            
                                            Parsing_that_DATA(data, Mysql_sharing_instance__CLONED, Agent_TcpStream_Inst_Mutex__with__Arc__CLONED );
                                        });
                                    }
                                    
                                    
                                },
                                None=>{
                                    /* 주로 타임아웃 시,,, */

                                    /* 1차 공유변수에서 TcpStream 관리 객체 제거. */
                                    Share_Agent_ID__and__TCP_stream.lock().unwrap().Remove_Agent_Data(&AGENT_ID_string); 
                                    drop(result_data);
                                    return;
                                }
                                
                            }
                        }
                        continue;

                    }
                    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                }
                /* 최종실패  */
                drop( Agent_TcpStream_Inst_Mutex__with__Arc );
                return

            },
            None=>{
                /* 최종실패  */
                drop( Agent_TcpStream_Inst_Mutex__with__Arc );
                return
            }
        }
    }else{
        /* 최종실패  */
        println!("Agent 와의 AGENT_ID 교류 전송 실패");
        drop(Agent_TcpStream_Inst_Mutex__with__Arc);
        return;
    }


    
}

fn Parsing_that_DATA(DATA:Vec<u8>, Mysql_sharing_instance: Arc<Mutex<MYSQL::DB_inst>>,Agent_TcpStream_Inst_Mutex__with__Arc: Arc<Mutex<TCP_STREAM_Instance>>){
    /*
    
        파싱은 어떻게 이루어지는가?.....
        {AGENT_ID (128바이트) } + { {TYPE 4바이트} + {길이 4바이트}+{실제데이터 가변바이트}      }
        
        1. TYPE 얻기
        2. Utils::Get_RAW_DATA_until__END 으로 동적 데이터 를 Vec< Vec<u8> > 습득

        3. DB저장

        4. TYPE업데이트 
    
    */
    let mut start_index:usize = 0 as usize;
    let mut last_index:usize = 128 as usize;

    let mut Agent_ID= [0u8;128];
    Agent_ID.copy_from_slice(&DATA[start_index..last_index]);
    println!("SHA값 -> {:?}", &DATA[start_index..last_index]);

    let Agent_ID = match std::str::from_utf8(&Agent_ID){
        Ok(Strings)=>{
            Strings.to_string()
        },
        Err(_)=>{return}
    };
    println!("에이전트SHA512->{}", Agent_ID);

    start_index = 128;
    last_index = 132;

    let mut TYPE: u32 = Utils::Bytes_to_U32_with_index(   &DATA,start_index,last_index ); //초기 넣어주기

    let save_Raw_Data_Size_or__END: usize = 0;//DATA_to_u32(&DATA,4,8) as usize; // 초기 길이 설정

    let RAW_DATA_ALL_SIZE:usize = DATA.len(); //받은 DATA전체 길이


    loop{
        /* 타입 확인하기 */
        TYPE;


        /* 파싱 하기 */
        let mut RAW_DATA_vec = Utils::Get_RAW_DATA_until__END(&mut start_index, &mut last_index, &DATA.to_vec());


        /* DB에 적용하기  */
        /*
            1. ENUM 값 확인하여 넣을 Table 확인.
            2. Utils.rs::ByteArrays_to_String 함수를 호출해서 String으로 변환 
            3. 테이블에 삽입하여 행 생성
        */
        
        let Strings_for_row_into_DB_table = Utils::ByteArray_to_String(&mut RAW_DATA_vec); // 테이블에 들어갈 각 칼럼들의 값을 위하여 Vec<u8>값을 String으로 변환함

        Insert_to_Table_for_AGENT(&Agent_ID, &TYPE, &Strings_for_row_into_DB_table, &Mysql_sharing_instance, &Agent_TcpStream_Inst_Mutex__with__Arc); // 드디어 DB 테이블에 저장
        

        drop(Strings_for_row_into_DB_table);

        /* 또 데이터가 있는 지 확인하기 *////////////
        if last_index == RAW_DATA_ALL_SIZE {
            /* 종료! */
            break;
        }
        /* 값 더 있으면 갱신 후 continue*/

        /* index를 다음 TYPE쪽으로 이동 */
        start_index = last_index;
        last_index = start_index + 4;

        /* 타입을 Update하고 바로 길이가 있는 index를 옮겨줘야함.  */
        TYPE = Utils::Bytes_to_U32_with_index(   &DATA,start_index,last_index  );// TYPE 갱신

        continue;
    }
   drop(Mysql_sharing_instance);
   println!("작업을 마칩니다..");

}







// agent_list 쿼리함 나중에 GUI 에서 대시보드 버튼을 만드는데 참조하는 Table임
fn Query_agent_list_Table(AGENT_ID:&String, Mysql_sharing_instance: &Arc<Mutex<MYSQL::DB_inst>>)->bool{

    let currnet_time_string:String = match Hashing::Get_Current_Timestamp_until_Micro(){
        Some(data)=>{
            data.0 as String
        },
        None=>{
            "None".to_string() as String
        }
    };

    let result = Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(format!("select AGENT_ID from agent_list where AGENT_ID = '{}' ", AGENT_ID).as_str() ); // 인수의 AGENT_ID가 DB에 먼저 있는 지 쿼리.
    if result.len() >= 1 {
        println!("Query_agent_list_Table -> {} AGENT_ID가 DB에 있습니다!", AGENT_ID);
    }else{
        println!("Query_agent_list_Table -> {} AGENT_ID가 DB에 없습니다! => 추가하겠습니다", AGENT_ID);
        Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(format!("INSERT INTO agent_list (AGENT_ID, timestamp) VALUES ('{}','{}' ) ", AGENT_ID, currnet_time_string).as_str() );
        //Create_New_Table_for_AGENT(AGENT_ID, Mysql_sharing_instance);
    }

    return true;
}


// AGENT_ID, 그리고 TYPE에 따라 Insert into 할 테이블을 고르고 Vec 스트링 DATA를 자동적으로 넣어주는 함수
fn Insert_to_Table_for_AGENT(
    AGENT_ID: &String,
    TYPE: &u32,
    Input_Strings_Vec_DATA: &Vec<String>,
    Mysql_sharing_instance: &Arc<Mutex<MYSQL::DB_inst>>,
    Agent_TcpStream_Inst_Mutex__with__Arc: &Arc<Mutex<TCP_STREAM_Instance>>
)->bool{
    //println!("Insert_to_Table_for_AGENT -> Input_Strings_Vec_DATA 의 값 -> {:?} @ TYPE -> {}", Input_Strings_Vec_DATA, TYPE);

    // 현재시간 
    let current_time= match Hashing::Get_Current_Timestamp_until_Micro(){
        Some(time)=>{
            time
        }
        None=>{
            ("None".to_string(), [0].to_vec())
        }
    };


    if *TYPE == TYPE::process_routine as u32 {
        /* process_routine 테이블에 삽입 */
        if Input_Strings_Vec_DATA.len() == 5 {
            /*
            [*]: AGENT_ID
            [0]: is_create
            [1]: pid
            [2]: full_path
            [3]: exe_size
            [4]: sha256
            */
            println!("Input_Strings_Vec_DATA-> {:?}",Input_Strings_Vec_DATA);
           
            Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(
                format!(
                    "INSERT INTO process_routine (AGENT_ID, is_create, pid, full_path, exe_size, sha256, DB_timestamp) VALUES ('{}', '{}', '{}', '{}', '{}', '{}','{}')",
                    AGENT_ID,Input_Strings_Vec_DATA[0].as_str(),Input_Strings_Vec_DATA[1].as_str(),Input_Strings_Vec_DATA[2].replace("\\", r"\\"),Input_Strings_Vec_DATA[3].as_str(),Input_Strings_Vec_DATA[4].as_str(),current_time.0
                ).as_str()
            );

            /* 고유한 파일인지 확인하고 file 테이블에 저장할 것인지 확인용 */
            INSERT_INTO_file_Table(&Input_Strings_Vec_DATA[4], Input_Strings_Vec_DATA[2].clone(),&Input_Strings_Vec_DATA[3],&Mysql_sharing_instance, &Agent_TcpStream_Inst_Mutex__with__Arc);

        }else if Input_Strings_Vec_DATA.len() == 3 {
            /* 대부분의 이 경우는 EXE바이너리를 못가져온 경우임 ㅋ  exe_size, sha256이 없는 것이다*/ 
            println!("EXE바이너리를 없음");
            Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(
                format!(
                    "INSERT INTO process_routine (AGENT_ID, is_create, pid, full_path, exe_size, sha256) VALUES ('{}', '{}', '{}', '{}', None, None)",
                    AGENT_ID,Input_Strings_Vec_DATA[0].as_str(),Input_Strings_Vec_DATA[1].as_str(),Input_Strings_Vec_DATA[2].replace("\\", r"\\")
                ).as_str()
            );
        }else{
            /* 이 경우는 HANDLE을 못얻었을 때 */
            println!("핸들 자체 없음");
        }
        
        

        
        return true;
    }
    else if *TYPE == TYPE::network_filter as u32 {
        /* network_filter 테이블에 삽입 */
        /*
            [0]: LOCAL_IP
            [1]: LOCAL_PORT
            [2]: REMOTE_IP
            [3]: REMOTE_PORT
            [4]: is_Inbound? (1byte)
            [5]: PID
            [6]: SHA256
            [7]: FULL-PATH
         */
        println!(" network_filter 받음!! -> {:?}", Input_Strings_Vec_DATA);
        
        return true;
    }
    else if(  *TYPE == TYPE::API_HOOKED_MON as u32 ){
        /*
            프로세스 API 후킹 결과 
        
            [0] PID 
            [1]  어드민 실행인가? (1 Byte) 
            
            [2]  프로그램 절대경로
            [3 ~ ]  파라미터 동적..  // 동적의 경우, for문으로 API 파라미터 저장 Table에 저장하거나, 허용가능한 수치만 저장 + 또는 , 기준으로 하나의 문자열로 변환 
            [?}] _END

         */
        println!("API_HOOKED_MON");

        // 동적 파라미터 를 하나의 문자열로.. 
        let mut Dynamic_Parameters = String::new();
        for (usize, String_Data) in Input_Strings_Vec_DATA.iter().enumerate() {


            if(usize>=3){
                // 동적 파라미터 구간  (동적 파라미터를 동적으로 하나의 문자열로 함)
                if(Dynamic_Parameters.len() < 1){
                    Dynamic_Parameters = format!("{}", String_Data );
                }else{
                    Dynamic_Parameters = format!("{}, {}", Dynamic_Parameters, String_Data );
                }
            }
        }

        Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(
            format!(
                "INSERT INTO process_api_hooked (AGENT_ID, pid, is_admin_running, path, parameters, timestamp ) VALUES ('{}', '{}', '{}', '{}', '{}', '{}')",
                AGENT_ID,Input_Strings_Vec_DATA[0].as_str(),Input_Strings_Vec_DATA[1].as_str(),Input_Strings_Vec_DATA[2].as_str(), Dynamic_Parameters.as_str(), current_time.0
            ).as_str()
        );
        return true;
    }
    else{
        return false;
    }
    

    
}




fn INSERT_INTO_file_Table(SHA256:&String, mut exe_full_path:String, exe_size:&String, Mysql_sharing_instance: &Arc<Mutex<MYSQL::DB_inst>>, Agent_TcpStream_Inst_Mutex__with__Arc: &Arc<Mutex<TCP_STREAM_Instance>>)->bool{

    /* 고유 SHA256만 담기는 file 테이블에 중복제외 삽입 시도. */
    let query_result = Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(
        format!(
            "SELECT File_SHA256 FROM file WHERE File_SHA256 = '{}' ",
            SHA256.as_str()
        ).as_str()
    );
    

    /* 고유한 SHA256만 삽입해야하므로, 결과가 전혀 없어야 함 ㅋ */
    if query_result.len() < 1 {
        
        /* 파일 가져오기 */
        exe_full_path = exe_full_path.replace("\\\\", r"\");


        let EXE_Program = match Agent_TcpStream_Inst_Mutex__with__Arc.lock().unwrap().SEND_DATA_with_SERVER_COMMAND(exe_full_path.as_bytes().to_vec(), SERVER_COMMAND::GET_file_bin){
            Some(data)=>{
                
                /* 한번더 file 테이블에서 중복인지 확인. */
                match Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(
                    
                    format!(
                        "SELECT File_SHA256 FROM file WHERE File_SHA256 = '{}' ",
                        SHA256.as_str()
                    ).as_str()
                ){
                    result=>{
                        if result.len() > 0 {
                            return true;
                        }
                    }
                }

                /* 바이너리 검증.  길이가 4면 No를 받았으므로 실패임 */
                if data.len()<=4 {
                    return false
                }else{
                    println!("EXE_Program길이-->{}",data.len());
                    data
                }
            },
            None=>{return false}
        };

        let current_time = Hashing::Get_Current_Timestamp_until_Micro().unwrap();//현재시간

        /* 가져온 파일 저장 */
        let SAVE_PATH = format!("{}_{}.exe", SHA256.as_str(),&current_time.0);
        let FULL_SAVE_PATH = match Utils::Save_to_Disk("C:\\saver", SAVE_PATH.as_str(), &EXE_Program){
            Some(FULL_SAVE_PATH)=>{
                println!("Save_to_Disk성공!@ -> {}", FULL_SAVE_PATH);
                FULL_SAVE_PATH
            },
            None=>return false
        };
        

        Mysql_sharing_instance.lock().unwrap().get_Query_from_MySql(
            format!(
                "INSERT INTO file (File_SHA256, signature, size, saved_path, DB_timestamp) VALUES ('{}','{}','{}','{}','{}') ",
                SHA256.as_str(), "exe", exe_size, FULL_SAVE_PATH.replace("\\",r"\\"),  current_time.0
            ).as_str()
        );
    }

    true
}