

use sha2::{Sha512, Digest};
use hex::encode;


pub fn Get_current_timestamp_and_SHA512(Salt_value: u32) ->Result<(Vec<u8>,String), &'static str>{
    
    //let current_timestamp_with_micro = Utc::now();

    //let mut testt = Vec::new();

    let mut current_server_local_time = Get_Current_Timestamp_until_Micro().unwrap();
    let current_server_local_time__Bytes = &mut current_server_local_time.1;
    let Salt:&[u8] = &Salt_value.to_le_bytes();
    
    current_server_local_time__Bytes.extend_from_slice(Salt);
    //let time_string = format!("{}", current_timestamp_with_micro);
    
    
    let mut hasher = Sha512::new();
    //hasher.update(time_string.as_bytes());
    hasher.update(current_server_local_time__Bytes);
    let result = hasher.finalize();


    let result_but_String = encode(result.clone()).to_string();
    let result_but_Bytes = result_but_String.as_bytes();
    //println!("최종 Agent_ID 생성 성공! -> 해시길이: {} 값: {}", result_but_Bytes.len(), result_but_String);
    if result_but_String.len()>= 128 && result_but_Bytes.len()>=128 {
        Ok((result_but_Bytes.to_vec(), result_but_String ))
    }else{
        Err("해시 실패!")
    }

    
}

/*
    현재 시간을 튜플로 반환
*/
pub fn Get_Current_Timestamp_until_Micro()->Option<(String,Vec<u8>)>{

    let current_time = chrono::Local::now();

    let timestamp = current_time.format("%Y-%m-%d-%H-%M-%S").to_string() + &format!(".{:06}", current_time.timestamp_subsec_micros());
    println!("현재 길이 -> {}", timestamp.len());
    Some( (timestamp.clone(), timestamp.as_bytes().to_vec()) )
}