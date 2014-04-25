<?php

// Note: have change php.ini to be able to upload larger files
// upload_max_filesize = 50M
// post_max_size = 192M

if ($_POST['demo_select_1'] != "") {
    $file1 = $_POST['demo_select_1'];
} else {
    $file1 = $_FILES['file_select_1']["tmp_name"];
}

if ($_POST['demo_select_2'] != "") {
    $file2 = $_POST['demo_select_2'];
} else {
    $file2 = $_FILES['file_select_2']["tmp_name"];
}

//expecting xcorrsound to be set up
$output = array();
//  	$command = "diff -s " . escapeshellarg(realpath($file1)) . " " . escapeshellarg(realpath($file2));
$command = $_POST['xcorrsoundFunction'] . " " . escapeshellarg(realpath($file1)) . " " . escapeshellarg(realpath($file2));
exec($command, $output);

$formatted = "";
foreach ($output as $line) {
    // append each line, but make it HTML-friendly first
    $formatted .= htmlspecialchars($line) . "<br>";
}

//	$result = json_encode( print_r($_POST, TRUE) . "<br />" . print_r($_FILES, TRUE) . "<br />Processed<br />" . $formatted . "<br />Command<br />" . $command);
$result = json_encode("<br />Processed<br />" . $formatted);
print($result);
?>