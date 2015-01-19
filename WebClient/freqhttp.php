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
		

		table {
			font-family: verdana,arial,sans-serif;
			font-size:11px;
			color:#333333;
			border-width: 1px;
			border-color: #666666;
			border-collapse: collapse;
			width:100%;
		}
		tr {
			border-width: 1px;
			padding: 8px;
			border-style: solid;
			border-color: #666666;
			background-color: #dedede;
		}
		td {
			border-width: 1px;
			padding: 8px;
			border-style: solid;
			border-color: #666666;
			background-color: #f0f0f0;
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
		
		#http_methods {
			xbackground: white 134px 0;
			xpadding: 1px 1em 2em 1em;
			background: white;
			padding: 25px 25px 25px 45px;
		}
		
		#http_methods_all {
			xbackground: white 134px 0;
			xpadding: 1px 1em 2em 1em;
			background: white;
			padding: 25px 25px 25px 45px;
		}
		
		#http_methods_frequency {
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
		
		<h3>Lista agentów:</h3>

		<?php
			//  echo "connecting to database\n"; 
		  
			$conn = new mysqli("localhost", "root", ""); 
			if ($conn->connect_error) {
				die("Connection failed: " . $conn->connect_error); 
			} 
			  
			  
			  
			echo "<table>";
			  
			      echo "<tr>";
				  echo " <td><b>ID sesji</td>";
				  echo " <td><b>ID klienta</b></td>";
				  echo " <td><b>Nr IP</b></td>";
			      echo "</tr>";
			      
			  
			     
			      $sql = "select * from tin.sessions"; 
			      $result = $conn->query($sql); 
			      if ($result == false) echo "Błąd zapytania sql"; 
			      else if ($result->num_rows > 0) {
				  while ($row = $result->fetch_assoc()) {
					  echo "<tr>";
					  echo "<td>".$row['id'] ."</td>";
					  echo "<td>".$row['client_id'] ."</td>";
					  echo "<td>".$row['client_ip'] ."</td>";
					 // ", client_id: " . $row['client_id'] . ", client_ip: " . $row['client_ip'] . "\n";
				  }
			      } else {
				  echo "Brak danych"; 
			      }
			      
			echo "</table>";

			  
		?>
		
		</div>
		
		
		<div id="http_methods_all" align="center">
		
		<h3>Wszytkie zapytania HTTP dla danej sesji:</h3>
		
		<form id="menu">
			  <label for="session_id">Numer sesji</label>
			  <input type="text" name="session_id" />
		</form>

		<?php
	  
			  
			echo "<table>";
			  
			      echo "<tr>";
				  echo " <td><b>Request ID</b></td>";
				  echo " <td><b>Receiver ID</b></td>";
				  echo " <td><b>Metoda HTTP</b></td>";
				  echo " <td><b>Czas</b></td>";
			      echo "</tr>";
			      
			      
			     
			      $sql = "select * from tin.requests where session_id = '".$_GET['session_id']."'"; 
			      $result = $conn->query($sql); 
			      if ($result == false) echo "Błąd zapytania sql"; 
			      else if ($result->num_rows > 0) {
				  while ($row = $result->fetch_assoc()) {
					  echo "<tr>";
					  echo "<td>".$row['id'] ."</td>";
					  echo "<td>".$row['receiver_ip'] ."</td>";
					  echo "<td>".$row['http_method'] ."</td>";
					  echo "<td>".$row['time'] ."</td>";
					
				  }
			      } else {
				  echo "Brak danych"; 
			      }
			      
			echo "</table>";

			  
		?>
		
		</div>
		
		
		<div id="http_methods" align="center">
		
		<h3>Suma różnych zapytań HTTP dla danej sesji:</h3>
		
		<?php
 
			  
			echo "<table>";
			  
			      echo "<tr>";
				  echo " <td><b>Request HTTP Method</b></td>";
				  echo " <td><b>Ilość rządań</b></td>";
			      echo "</tr>";
			      
			      
			     
			      $sql = "select http_method, count(*) as req_number from tin.requests where session_id = '".$_GET['session_id']."'group by http_method"; 
			      $result = $conn->query($sql); 
			      if ($result == false) echo "Błąd zapytania sql"; 
			      else if ($result->num_rows > 0) {
				  while ($row = $result->fetch_assoc()) {
					  echo "<tr>";
					  echo "<td>".$row['http_method'] ."</td>";
					  echo "<td>".$row['req_number'] ."</td>";
					
				  }
			      } else {
				  echo "Brak danych"; 
			      }
			      
			echo "</table>";

			  
		?>
		
		</div>
		
		<div id="http_methods_frequency" align="center">
		
		<h3>Częstotliwość zapytań HTTP dla danej sesji:</h3>
		
		<?php
 
			  
			echo "<table>";
			  
			      echo "<tr>";
				  echo " <td><b>Request HTTP Method</b></td>";
				  echo " <td><b>Ilość zapytań na minutę</b></td>";
			      echo "</tr>";
			      
			      //echo "lalala";
			      
			      $sql_all = "select http_method, count(*) as req_number from tin.requests where session_id = '".$_GET['session_id']."'group by http_method"; 
			      $result_all = $conn->query($sql_all);
			      if ($result_all == false) echo "Błąd zapytania sql"; 
			      else if ($result_all->num_rows > 0) {
					//echo "lalala2";
					$sql = "select * from tin.requests where session_id = '".$_GET['session_id']."'order by time"; 
					$result = $conn->query($sql); 
					$array = array();
					if ($result == false) echo "Błąd zapytania sql"; 
					else if ($result->num_rows > 0) {
					      while ($row = $result->fetch_assoc()) {
						  array_push($array, $row['time']);
					      }
					      $n = count($array);
					      $first_time = $array[0];
					     // echo $first_time;
					      
					      $last_time = $array[$n - 1];
					     //  echo $last_time;
					      
					      $difference = abs($last_time - $first_time);
					      $minutes   = round($difference / 60);
					      //echo $minutes;
					      
					      
					      /*while ($row = $result_all->fetch_assoc()) {
							  echo "<tr>";
							  echo "<td>".$row['http_method'] ."</td>";
							//  $number = intval($row['req_number']);
							 // echo $number;
							 // echo "<td>".($row['req_number']/$minutes) ."</td>";
					
						    }
					      } else {
						    echo "Brak danych"; 
					      }*/
					      
					     // echo $difference;
					      
					      
					//  $first_time = $result[0].$row['session'];
                                          //echo "lala";
					//  echo $first_time;
					//  $last_time = $result[num_rows].$row['time'];
					/*  
					  $difference = intval($last_time - $first_time);
					  echo $difference;
					    while ($row = $result->fetch_assoc()) {
						    echo "<tr>";
						    echo "<td>".$row['http_method'] ."</td>";
						//    echo "<td>".intval($row['req_number']) / $difference."</td>";
						  
					    }*/
					} else {
					    echo "Brak danych"; 
					}
			      }
			      
			echo "</table>";

			  
		?>
		
		</div>


			 
		<?php	  
			  
		  
			  $conn->close(); 
		
		?>
		  
		
		
	</div>


</body>

</html>