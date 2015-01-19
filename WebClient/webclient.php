<?php
	
	function send($msg, $length) {
		global $write_fifo; 
		echo $msg . "\n"; 
		if ( fwrite($write_fifo, $msg, $length) == false ) echo "Error sending message\n"; 
	}
	
	
	function send_start_or_stop($client_id, $date, $start_or_stop) {
		if ($client_id < 10) $c = "0" . strval($client_id); 
		else $c = strval($client_id); 
		
		$msg = $start_or_stop . " " . $c . " " . $date->getTimestamp(); 
		if (strlen($msg) != 15) {
			echo "start message wrong length!\n"; 
			return; 
		}
		
		send($msg, 15); 
	}
	
	function send_start($client_id, $date) {
		send_start_or_stop($client_id, $date, "1"); 
	}
	
	function send_stop($client_id, $date) {
		send_start_or_stop($client_id, $date, "2"); 
	}	
	
	function send_get_data($client_id) {
		if ($client_id < 10) $c = "0" . strval($client_id); 
		else $c = strval($client_id); 
		$msg = "3 " . $c; 
		send($msg, 4); 
	}
	
	function send_list_clients() {
		send("4", 1); 
	}
	
	function recv_response($length) {
		global $read_fifo; 
		$result = fread($read_fifo, $length); 
		if ($result == false) {
			echo "error reading response\n"; 
		}
		return $result; 
	}
	
	echo "no elo\n"; 
	
	$fifo1_name = "/tmp/fifo1";
	$fifo2_name = "/tmp/fifo2"; 
	
	$write_fifo = fopen($fifo1_name, "w+");
	if ($write_fifo == false) echo "Cannot open fifo1\n"; 
	
	echo "write_fifo opened\n"; 
	
	$read_fifo = fopen($fifo2_name, "r+"); 
	if ($read_fifo == false) echo "Cannot open fifo2\n"; 
		
	echo "read_fifo opened\n"; 
	
	date_default_timezone_set("Europe/Warsaw"); 
	
	/*
	// wysyłanie start i odbieranie odpowiedzi
	send_start(1, new DateTime()); 
	$response = recv_response(1); 
	if ($response == "1") echo "start response: OK\n"; 
	else echo "start response: ERROR\n"; 
	
	// stop
	send_stop(2, new DateTime("2015-01-23 15:00:15")); 
	$response = recv_response(1); 
	if ($response == "1") echo "stop response: OK\n"; 
	else echo "stop response: ERROR\n"; 
	
	
	// get_data
	send_get_data(3); 
	$response = recv_response(1); 
	if ($response == "1") echo "get_data response: OK\n"; 
	else echo "get_data response: ERROR\n"; 
	
	// list_clients
	send_list_clients(); 
	$client_num = intval(recv_response(2)); 
	echo "client_num: " . $client_num . "\n"; 
	for ($i = 1; $i <= $client_num; $i++) {
		echo $i . ": " . recv_response(20); 
	}
	*/
	
	// podłączanie się do bazy
	echo "connecting to database\n"; 
	
	$conn = new mysqli("localhost", "root", ""); 
	if ($conn->connect_error) {
		die("Connection failed: " . $conn->connect_error); 
	} 
	
	echo "connected successfully\n";
	
	$sql = "select * from tin.sessions"; 
	$result = $conn->query($sql); 
	if ($result == false) echo "Error executing sql"; 
	else if ($result->num_rows > 0) {
		while ($row = $result->fetch_assoc()) {
			echo "session id: " . $row['id'] . ", client_id: " . $row['client_id'] . ", client_ip: " . $row['client_ip'] . "\n";
		}
	} else {
		echo "nie mam Ci nic do wyswietlenia"; 
	}
	
	$sql = "select * from tin.requests where session_id = 8"; 
	$result = $conn->query($sql); 
	if ($result == false) echo "Error executing sql"; 
	else if ($result->num_rows > 0) {
		while ($row = $result->fetch_assoc()) {
			echo "request id: " . $row['id'] . ", receiver_ip: " . $row['receiver_ip'] . ", http_method: " . $row['http_method'] . ", time: " . $row['time'] . "\n";
		}
	} else {
		echo "nie mam Ci nic do wyswietlenia"; 
	}
	
	$sql = "select http_method, count(*) as req_number from tin.requests where session_id = 10 group by http_method"; 
	$result = $conn->query($sql); 
	if ($result == false) echo "Error executing sql"; 
	else if ($result->num_rows > 0) {
		while ($row = $result->fetch_assoc()) {
			echo "request http_method: " . $row['http_method'] . ", req_number: " . $row['req_number'] . "\n";
		}
	} else {
		echo "nie mam Ci nic do wyswietlenia"; 
	}
	
	
	$conn->close(); 
	
	
?>

