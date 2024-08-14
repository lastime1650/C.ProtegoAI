use std::thread::{self};
use std::net::{TcpListener};

use std::sync::{Mutex,Arc};

use crate::Agent_1vs1_Manager;
use crate::MYSQL;
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
                     Agent_1vs1_Manager::Start_Agent(
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
/* main() -> here -> Agent_1vs1_Manager()  */