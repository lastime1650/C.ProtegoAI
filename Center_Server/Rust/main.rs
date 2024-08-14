

use std::sync::{Mutex,Arc};

use std::thread;

/*
    TCP 서버 이며, 여러 Client와 상호작용한다.
    단일 포트로 데이터 및 명령을 송수신 하도록 한다.
*/
mod Utils;
mod Agent_1vs1_Manager;
mod LISTENING_;
mod MYSQL;
mod AGENT_MANAGEMENT_functions;
mod Hashing;
mod communication_GUI_program_for_connection;
mod Share_TCP_and_AGENT_ID; // GUI 와 Agent 간 스레드의 공유자원이다. GUI에서 특정 Agent의 명령을 내리기 위한, 공유 데이터인 셈이다.
mod connect_AI_analyser; // AI 분석 서버와 연결함

use crate::MYSQL::DB_inst_METHODS;
fn main() {

    /* 
    //마이크로초까지 타임스탬프 추출
    let DATA = Hashing::Get_current_timestamp_and_SHA512().unwrap();
    println!(" {:?} , {} ", DATA.0, DATA.1);
    */
    //println!("{:?}", Hashing::Get_current_timestamp_and_SHA512(0xFFFF).unwrap().0);
   
   //MYSQL::DB_insert();
   let mysql_: MYSQL::DB_inst = MYSQL::DB_inst_METHODS::new("mysql://root:tjddnjs12@localhost:3306/process_db".to_string());
   /*
   
        공유자원 생성

    
    */
    // MySql
    let TMP_Mysql_sharing_instance = Mutex::new(mysql_);
    let Mysql_sharing_instance: Arc<Mutex<MYSQL::DB_inst>> = Arc::new(TMP_Mysql_sharing_instance);
    /* 
    // SHare Agent
    */

    let Share_Agent_ID__and__TCP_stream = Share_TCP_and_AGENT_ID::AGENT_GUI_share::new();
    let Share_Agent_ID__and__TCP_stream = Mutex::new(Share_Agent_ID__and__TCP_stream);
    let Share_Agent_ID__and__TCP_stream = Arc::new(Share_Agent_ID__and__TCP_stream);

   

    
    //공유자원 복사 후 두 병렬 스레드에 전달 
    let mut CLONED = Arc::clone(&Mysql_sharing_instance); // Mysql 
    let mut Share_CLONED = Arc::clone(&Share_Agent_ID__and__TCP_stream); // Share Agent_ID
    // 실제 AGENT들과 통신하는 TCP 
    thread::spawn( move || {  LISTENING_::Start_Listen("0.0.0.0:1029".to_string(), CLONED, Share_CLONED); }  );
    


     
    //let mut CLONED2 = Arc::clone(&Mysql_sharing_instance);
    //let mut Share_CLONED2 = Arc::clone(&Share_Agent_ID__and__TCP_stream); // Share Agent_ID
    /* GUI로 AGENT를 관리하는 TCP */
    //thread::spawn( move || {  communication_GUI_program_for_connection::Start_Listen("0.0.0.0:10299".to_string(), CLONED2, Share_CLONED2); }  );
   
    //let CLONED3 = Arc::clone(&Mysql_sharing_instance);
    //connect_AI_analyser::Start_Connect_to_AI_analyser_Server(CLONED3,"b081a247f8bde7d98337dec2a44b9dff7d1eba19bd99a20c04395cea831177ee".to_string()); // loop문 대체
    loop{}


}
