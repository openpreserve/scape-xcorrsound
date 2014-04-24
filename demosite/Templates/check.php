 <?php 



require_once "../StyleJS/json/JSON.php";
$json = new Services_JSON();

$url1 = $_GET["url1"];
$url2 =  $_GET["url2"];

$descriptorspec = array(
    0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
    1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
    2 => array("pipe", "w")   // 2 is STDERR for process
 );

$cmd = "DISPLAY=:0 java -Djava.awt.headless=true -jar ./demo3.jar ".$url1." ".$url2." &";

$p=proc_open ($cmd, $descriptorspec, $pipes);

$out = stream_get_contents($pipes[1]) ;

$out2 = $json->encode($out);
$parts = explode('\n', $out2);

$result = $json->encode($parts[sizeof($parts)-1]);
print($result);

proc_close($p);


?>
