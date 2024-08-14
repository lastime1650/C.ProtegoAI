
use std::sync::{Mutex,Arc};

use crate::AGENT_MANAGEMENT_functions::{TCP_STREAM_Instance}; // Agent TCP 스트림 전체를 관리하는 인스턴스를 가져옴

pub struct AGENT_GUI_share{
    AGENT_info:Option<Vec<(String, Arc<Mutex< TCP_STREAM_Instance >>)>> // Agent_ID , 에이전트 TcpStream 인스턴스 (뮤텍스 필수!)
}

impl AGENT_GUI_share{
    pub fn new()->Self{
        AGENT_GUI_share{
            AGENT_info:None
        }
    }

    pub fn Insert_Agent_Data(&mut self, Agent_ID:& String, Agent_TCP_STREAM: &mut Arc<Mutex<TCP_STREAM_Instance>>){
        match self.AGENT_info.as_mut(){
            Some( n)=>{

                for (index, DATA) in n.iter_mut().enumerate(){
                    if DATA.0 == *Agent_ID{
                        println!("이미 중복된 Agent_ID를 넣으려고 했기에, 실패! ");
                        return
                    }
                }
                
                n.push((Agent_ID.clone(), Arc::clone(Agent_TCP_STREAM) ));
            },
            None =>{ 
                let mut init:Vec<(String,Arc<Mutex< TCP_STREAM_Instance >>)> = Vec::new();
                init.push((Agent_ID.clone(), Arc::clone(Agent_TCP_STREAM)  ));
                self.AGENT_info = Some(init);
            }
        }
    }

    pub fn Remove_Agent_Data(&mut self, Agent_ID:&String)->bool{
        match self.AGENT_info.as_mut(){
            Some(n)=>{

                for (index, DATA) in n.iter_mut().enumerate(){
                    if DATA.0 == *Agent_ID{
                        n.remove(index);
                        return true;
                    }
                }

            },
            None=>{println!("Remove_Agent_Data 제거할 것이 아예없습니다.");}
        }
        return false;
    }

    pub fn Get_TCP_Stream_from_Agent_ID(&mut self , Agent_ID:&String)->Option<Arc<Mutex<TCP_STREAM_Instance>>>{
        match self.AGENT_info.as_mut(){
            Some(n)=>{

                for (index, DATA) in n.iter().enumerate(){
                    if DATA.0 == *Agent_ID{
                        return Some(Arc::clone( &DATA.1 ));
                    }
                }
                return None;

            },
            None=>{println!("[Get_TCP_Stream_from_Agent_ID] 현재 연결 중인 Agent를 찾을 수 없습니다."); return None}
        }
    }
}