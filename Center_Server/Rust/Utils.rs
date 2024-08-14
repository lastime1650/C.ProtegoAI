/*
    변환 함수가 모인 공간.
*/

use std::io::Write;



pub fn Bytes_to_U32(bytes: &[u8])->u32{

    let mut sample = [0u8;4];
    sample.copy_from_slice(bytes);
    u32::from_le_bytes(sample)
}


pub fn Bytes_to_U32_with_index(Sliced_DATA:&[u8],start_index:usize, end_index:usize)->u32{
    
    /* Sliced_DATA 의 길이가 4라면, ULONG32 숫자 취급 */
    if Sliced_DATA.len() >= 4 {
        let mut tmp = [0u8;4];
        tmp.copy_from_slice(&Sliced_DATA[start_index..end_index]);
        
        return u32::from_le_bytes(tmp);
    }else{
        0
    }

}

/* 아스키 문자열 ==> String */
pub fn asciiByte_2_String( DATA:&[u8] )->String{
    std::str::from_utf8(DATA).unwrap().to_string()
}


/* 각종 길이 타입을 ==> String */
fn ulongByte_2_String(    DATA:&[u8]   )->String{
    if DATA.len() == 4 { // 32(4B)

        let mut tmp_4_byte = [0u8;4];
        tmp_4_byte.copy_from_slice(DATA);

        let tmp_u32 = u32::from_le_bytes(tmp_4_byte);
        tmp_u32.to_string() 

    }else if DATA.len() == 8 { // 64(8B)
        let mut tmp_4_byte = [0u8;8];
        tmp_4_byte.copy_from_slice(DATA);
        
        let tmp_u64 = u64::from_le_bytes(tmp_4_byte);
         tmp_u64.to_string() 
    }else if DATA.len() == 1 { // 1
        /* boolean */
        if DATA[0..1] == [0x0]{
            "False".to_string()
        }else{
            "True".to_string()
        }
    }
    else if DATA.len() == 16 { // 128(16B)
        let mut tmp_16_byte = [0u8;16];
        tmp_16_byte.copy_from_slice(DATA);
        let tmp_u128:u128 = u128::from_le_bytes(tmp_16_byte);
        tmp_u128.to_string()
    }
    else{
        "Can't Convert".to_string()
    }
}


// Vec< Vec<u8> > 을 입력받았을 때, 무조건 String으로 변환 (정수의 경우,,, u8->정수형->String)
pub fn ByteArray_to_String(INPUT_DATA:&mut Vec<Vec<u8>>)->Vec<String>{

    let mut return_string_array:Vec<String> = Vec::new();
    for data in INPUT_DATA.iter(){
        
        let mut TMP_String:String = String::new();
        
        // from_utf8로 문자열 가능한지 확인.
        TMP_String = match String::from_utf8(data.clone()){
            Ok(get_String)=>{
                if data.len() == 4 {
                    // 32bit ULONG32
                    format!("{}", u32::from_le_bytes(data[0..4].try_into().unwrap()))
                }
                else if data.len() == 8 {
                    // 64bit ULONG64
                    format!("{}", u64::from_le_bytes(data[0..8].try_into().unwrap()))
                }else if data.len() == 1 {
                    // bool BOOLEAN
                    if *data == [0].to_vec(){
                        format!("0")
                    }else if *data == [1].to_vec(){
                        format!("1")
                    }else{
                        format!("{:?}", data.as_slice())
                    }
                }
                else{
                    // 결론적으로 정수가 아닐 때 그대로 반환
                    format!("{}",get_String)
                }
            },
            Err(_)=>{
                 // 문자열 한번에 변환 실패
                 // 정수형으로 변환 (길이기준)하여 String으로 format! 
                if data.len() == 4 {
                    // 32bit ULONG32
                    format!("{}", u32::from_le_bytes(data[0..4].try_into().unwrap()))
                }
                else if data.len() == 8 {
                    // 64bit ULONG64
                    format!("{}", u64::from_le_bytes(data[0..8].try_into().unwrap()))
                }else if data.len() == 1 {
                    // bool BOOLEAN
                    if *data == [0].to_vec(){
                        format!("0")
                    }else if *data == [1].to_vec(){
                        format!("1")
                    }else{
                        format!("{:?}", data.as_slice())
                    }
                }
                else{
                    // 싹 다 실패 UNKNOWN
                    format!("{:?}", data.as_slice())
                }
                
            }
        };
        //println!("\nByteArray_to_String-> data -> {:?} / String 처리후 -> {}\n", data, TMP_String);
        return_string_array.push(TMP_String.clone());
       

    }
    return_string_array
}




// ENUM 과 _END 사이의 동적인 데이터를 자동으로 수집하여 Vec<Vec<u8>> 로 Output해주는 함수
// start_index는 enum값 마지막 index를 넘겨주면, 시작 시 4바이트 길이로 넘어가 처리함.
pub fn Get_RAW_DATA_until__END( start_index:&mut usize, last_index:&mut usize, Parsing_Data:&Vec<u8> )->Vec<Vec<u8>>{
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
        //let mut tmp_Raw_DATA:Vec<u8> = Vec::new();
        *start_index = *last_index;
        *last_index = *start_index + Raw_DATA_len as usize;
        //tmp_Raw_DATA.extend(&Parsing_Data[*start_index..*last_index]);
        //println!("Raw_DATA -> {:?}", tmp_Raw_DATA);

        //Output_Vector.push(tmp_Raw_DATA.clone()); // 전역변수에 축적! 

        Output_Vector.push( Parsing_Data[*start_index..*last_index].to_vec().clone() );


    }
    return Output_Vector;
}

pub fn APPEND_DATA__into__send_data( SEND_DATA:&mut Vec<u8> , data: &[u8] ){

    let data_len_BYTEs = u32::to_le_bytes(data.len() as u32);
    SEND_DATA.extend(data_len_BYTEs.clone());

    SEND_DATA.extend(data);

    return;

}

pub fn Save_to_Disk(Absolute_Dir_PATH:&str, File_Name:&str, Binary:&Vec<u8> )->Option<String>{

    // 디렉터리가 없으면 생성
    if !std::path::Path::new(Absolute_Dir_PATH).exists() {
        std::fs::create_dir_all(Absolute_Dir_PATH).unwrap();
    }
    let FULL_SAVE_PATH = format!("{}\\{}", Absolute_Dir_PATH, File_Name);
    // 파일을 디렉터리에 생성
    let mut file = match std::fs::File::create(FULL_SAVE_PATH.as_str()) {
        Err(why) => {
            panic!("couldn't create {}: {}", FULL_SAVE_PATH.as_str(), why);
            return None;
        },
        Ok(file) => file,
    };

    // 바이너리 데이터를 파일에 씁니다
    match file.write_all(Binary) {
        Err(why) => {
            panic!("couldn't create {}: {}", FULL_SAVE_PATH.as_str(), why);
            None
        },
        Ok(_) => {
            println!("successfully wrote to {}", FULL_SAVE_PATH.as_str());
            Some( FULL_SAVE_PATH )
        }
    }

}

pub fn Load_from_Disk(FULL_SAVE_PATH:&String)->Option<Vec<u8>>{
    match std::fs::read(FULL_SAVE_PATH){
        Ok(binary)=>{
            Some(binary)
        },
        Err(_)=>{println!("파일을 가져올 수 없습니다;;"); None}
    }
}

pub fn Remove_File_in_Disk(FULL_SAVE_PATH:&String)->bool{
    match std::fs::remove_file(FULL_SAVE_PATH){
        Ok(n)=>{
            println!("파일 삭제됨");
            true
        },
        Err(_)=>{
            false
        }
    }
}