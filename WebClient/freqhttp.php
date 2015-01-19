	<?php
		
		
	function send($msg, $length) {
		global $write_fifo; 
		if ( fwrite($write_fifo, $msg, $length) == false ) echo "Błąd przy wysyłaniu wiadomości\n"; 
	}
	
	
	function send_start_or_stop($client_id, $date, $start_or_stop) {
		if ($client_id < 10) $c = "0" . strval($client_id); 
		else $c = strval($client_id); 
		
		$msg = $start_or_stop . " " . $c . " " . $date->getTimestamp(); 
		if (strlen($msg) != 15) {
			echo "Wiadomość początkowa o złym rozmiarze!\n"; 
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
			echo "Błąd przy czytaniu odpowiedzi\n"; 
		}
		return $result; 
	}
	?>

<!DOCTYPE html>

<html>
<head>
	<meta charset="utf-8">
	<title>FreQHttp</title>
	<style>
	
		body {
			font: 80% arial, helvetica, sans-serif;
			background: #ccc;
			margin: 0;
		}
		
		.accessaid, h1 {
			position: absolute;
			height: 0;
			overflow: hidden;
		}
		
		#header {
			background: #036 url(images/obrazek2.gif);
			height: 60px;
		}
		
		
		h2 {
			font-size: 1.5em;
			color: #036;
		}
		
		h3 {
			font-size: 1.1em;
			color: red;
		}
		
		#container {
			position: relative;
			width: 1000px; /* width + border for IE 5.x */
			border: solid #036;
			border-width: 12 3px;
			margin: auto;
		}
		
		#console {
			xbackground: white 134px 0;
			xpadding: 1px 1em 2em 1em;
			background: white;
			padding: 25px 25px 25px 45px;
			border:dotted #036;
			border-width: 12 3px;
		}
		
		#menu {
			xbackground: white 134px 0;
			xpadding: 1px 1em 2em 1em;
			background: white;
			padding: 25px 25px 25px 25px;
		}
		
		#users_table {
			xbackground: white 134px 0;
			xpadding: 1px 1em 2em 1em;
			background: white;
			padding: 25px 25px 25px 45px;
		}
		

	</style>

</head>

<body>



	<div id="container">

		<div id="header">
			<h1>FreQHttp</h1>
		</div>

		<div>
		</div>
		
		<div id="menu" align="center">
		
			<form>
				<label for="agent_id">Id agenta</label>
				<input type="text" name="agent_id" />
				<input type="submit" class="button" name="start" value="Start pomiaru" />
				<input type="submit" class="button" name="stop" value="Stop pomiaru" />
				<input type="submit" class="button" name="get_data" value="Pobierz dane" />
				<input type="submit" class="button" name="list_clients" value="Lista agentów" />
			</form>

			
		</div>

		<div id="console" align="center">
		
			<p> <h3>Komunikaty:</h3> </p>
			<p> <?php

			$fifo1_name = "/tmp/fifo1";
			$fifo2_name = "/tmp/fifo2"; 
			
			$write_fifo = fopen($fifo1_name, "w+");
			if ($write_fifo == false) echo "Błąd. Nie można otworzyć fifo1\n"; 
			
			$read_fifo = fopen($fifo2_name, "r+"); 
			if ($read_fifo == false) echo "Błąd. Nie można otworzyć fifo2\n"; 
			
			date_default_timezone_set("Europe/Warsaw"); 
			
			if($_GET){
			    if(isset($_GET['start']) && !empty($_GET['agent_id']) ){
			    
				send_start($_GET['agent_id'],new DateTime());
				$response = recv_response(1); 
				if ($response == "1") echo "Odpowiedź: OK\n"; 
				else echo "Odpowiedź: ERROR\n"; 
				
			    }elseif(isset($_GET['stop']) && !empty($_GET['agent_id'])){
			    
				send_stop($_GET['agent_id'], new DateTime());
				$response = recv_response(1); 
				if ($response == "1") echo "Odpowiedź: OK\n"; 
				else echo "Odpowiedź: ERROR\n"; 
			    
			    }
			    elseif(isset($_GET['get_data']) && !empty($_GET['agent_id'])){
			
				send_get_data($_GET['agent_id']);
				$response = recv_response(1); 
				if ($response == "1") echo "Odpowiedź: OK\n"; 
				else echo "Odpowiedź: ERROR\n"; 
			    
			    }
			    elseif(isset($_GET['list_clients'])){
				
				send_list_clients();
				$client_num = intval(recv_response(2)); 
				echo "Lista agentów: " . $client_num . "\n"; 
				for ($i = 1; $i <= $client_num; $i++) {
					echo $i . ": " . recv_response(20); 
				}
			    
			    }
			}
	
			?> </p>
		
		</div>
		
		<div id="users_table" align="center">
		
		<h3>Lista agentów </h3>
		
		<?php
			//  echo "connecting to database\n"; 
		  
			  $conn = new mysqli("localhost", "root", ""); 
			  if ($conn->connect_error) {
				  die("Connection failed: " . $conn->connect_error); 
			  } 
			  
			 // echo "connected successfully\n";
			  
			  $conn->close(); 
		
		?>
		  
		
		</div>

	</div>


</body>

</html>