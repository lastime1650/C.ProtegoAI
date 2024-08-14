
use mysql::*;
use mysql::prelude::*;

use crate::Hashing;

/* 
pub fn DB_insert()->Result<()>{
    let url = "mysql://root:Tjddnjs!650@localhost:3306/process_db";
    let pool = mysql::Pool::new(url)?;
    println!("{:?}", pool);
    let mut conn = pool.get_conn()?;
    println!("{:?}", conn);
    conn.exec_drop(r"INSERT INTO test (c1) VALUES (:c1)",
    params!{
        "c1" => "korea",
    },)?;
    Ok(())
}
*/


/* TYPE 유형 */



pub struct DB_inst{
    DB_url: String,
    DB_pool_connection_inst: PooledConn//DB 연결 성공 시, 생성되는 인스턴스
}

pub trait DB_inst_METHODS{
    fn new(DB_url:String)->Self;

    
    fn get_Query_from_MySql(&mut self, Query_str:&str)-> Vec<Vec<String>> ; // str 받으면 쿼리를 해줌 !

    fn Query_file(&mut self, FILE_SHA256:String , File_save_path:&String ,FILE_size:String);// 공용... Agent들이 보낸 고유한 데이터를 모으는 곳. 이미 존재한 SHA256이라면 무시깜.


    fn Query_connecting_gui(&mut self,GUI_ID : &String, is_request_of_Disconnecting: bool) -> bool;

    fn Execute_Query_Drop(&mut self, Query_str:&str)->bool;
}

impl DB_inst_METHODS for DB_inst{
    fn new(DB_url:String)->Self{

        let pool = mysql::Pool::new(DB_url.as_str()).unwrap();
        let conn = pool.get_conn().unwrap();
        
        DB_inst{
            DB_url: DB_url,
            DB_pool_connection_inst: conn
        }
        
        
    }
    



    fn get_Query_from_MySql(&mut self, Query_str:&str)-> Vec<Vec<String>> 
    {
            let result:Vec<mysql::Row> = self.DB_pool_connection_inst.query(Query_str).unwrap();
            // query 메서드 결과 -> Row { AGENT_ID: Bytes("1"), DB_timestamp: Bytes("2024") } 이렇게 나옴

            let data: Vec<Vec<String>> = result.into_iter().map(|row| {
                // 각 Row의 모든 컬럼 값을 순회하며 Vec<String>으로 변환
                row.unwrap().iter().map(|value| {
                    match value{
                        Value::NULL => "NULL".to_string(),
                        Value::Bytes(ref x) => String::from_utf8_lossy(x).into_owned(),
                        Value::Int(x) => x.to_string(),
                        Value::UInt(x) => x.to_string(),
                        Value::Float(x) => x.to_string(),
                        Value::Double(x) => x.to_string(),
                        Value::Date(year, month, day, hour, minute, second, microsecond) => {
                            format!("{}-{}-{} {}:{}:{}.{}", year, month, day, hour, minute, second, microsecond)},
                        // 다른 Value 타입들에 대해서도 필요에 따라 처리
                        _ => {"타입이 지원되지 않음!".to_string()}
                    }
                }).collect() //2차원
            }).collect(); // 1차원

            data

             //println!("query 테스트 -> {:?}", data);
        
        
    }

    /* file 테이블을 확인하며, 모든 Agent로부터 얻은 데이터를 고유하게 저장한다. (전역적인 정적/동적/AI용으로..) */
    /* 아래 함수 호출은 비동기 처리 요구됨! */
    /*
        최초로 넣는 경우, 특정 절대경로에 SHA256을 이름으로 해서, HDD에 저장한다.
     */
    fn Query_file(&mut self, FILE_SHA256:String ,File_save_path:&String ,FILE_size:String){
        /* DB 쿼리 날리기 */
        let test:Vec<String> = self.DB_pool_connection_inst.query_map(
            "SELECT DISTINCT File_SHA256 from file", // DISTINCT 는 중복제외 쿼리 
            |get_FILEs_SHA256|(get_FILEs_SHA256),
        ).unwrap();

        for (index, get_SHA256) in test.iter().enumerate(){
            /* 있으면 바로 리턴 */
            if *get_SHA256 == FILE_SHA256{
                println!("Query_file -> SHA256중복으로 바로 Return 됨!");
                return
            }
        }
        /* 없으면 INSERT */
        let File_save_path = format!("C:\\{}_{}", FILE_SHA256,FILE_size); // <= 고유하게 얻은 파일을 물리적으로 저장하며, 그 경로를 DB에 저장
        //let File_save_path = format!("{}", File_save_path);
        /* 최초 삽입하기. */
        self.DB_pool_connection_inst.exec_drop( r"INSERT INTO file (File_SHA256, signature, size, saved_path, DB_timestamp) VALUES (:c1 , :c2, :c3, :c4, :c5)",
        params!{
            "c1" => FILE_SHA256.as_str(),
            "c2" => "Unknown",//시그니처 구현해야함;; ( parsing 함수에서 하자! )
            "c3" => FILE_size,//ulongByte_2_String(&FILE_size.to_le_bytes()).as_str(),
            "c4" => File_save_path.as_str(),
            "c5" => Hashing::Get_Current_Timestamp_until_Micro().unwrap().0.as_str()
            },
        );
        /* 리턴 */
    }

    fn Query_connecting_gui(&mut self,GUI_ID : &String, is_request_of_Disconnecting: bool) -> bool{
        /* 쿼리하기 */
        let test:Vec<String> = self.DB_pool_connection_inst.query_map(
            "SELECT DISTINCT GUI_ID from connecting_guis", // DISTINCT 는 중복제외 쿼리 
            |GUI_ID|(GUI_ID),
        ).unwrap();

        println!("{:?}", test);

        /* 쿼리 반환 값을 for문으로 확인하기 */
        for (index, get_current_Agent_ID_data) in test.iter().enumerate(){
            if get_current_Agent_ID_data == GUI_ID{

                if is_request_of_Disconnecting {
                    /* 정상적으로 삭제하기. */
                    self.DB_pool_connection_inst.exec_drop(r"DELETE FROM connecting_guis WHERE GUI_ID = :c1",
                params!{
                    "c1" => GUI_ID.as_str()
                });
                    
                    return true;
                }
                /* 이미 연결 중이므로 리턴 */
                println!("Query_connecting_agent => 이미 연결 중임!");
                return true; // 이미 연결 중임. 
            }
        }
        
        if is_request_of_Disconnecting {
            println!(" Agent 연결해제를 받았지만, DB에는 해당 연결된 AGENT_ID가 없어, 삭제가 불가능합니다. ");
            return false;
        }
        /* 최초 삽입하기. */
        self.DB_pool_connection_inst.exec_drop( r"INSERT INTO connecting_guis (DB_timestamp, GUI_ID) VALUES (:c1 , :c2)",
        params!{
            "c1" => Hashing::Get_Current_Timestamp_until_Micro().unwrap().0.as_str(),
            "c2" => GUI_ID.as_str()
            },
        );
        return true; // 성공반환
    }

    // 단순 쿼리만 할 때,
    fn Execute_Query_Drop(&mut self, Query_str:&str)->bool{

        match self.DB_pool_connection_inst.query_drop( Query_str){
            Ok(success)=>{true},
            Err(_)=>{false}
        }
    }
}

