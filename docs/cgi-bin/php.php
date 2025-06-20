<?php
echo "Content-Type: text/plain\r\n";
echo "\r\n";

echo "PHP CGI Script Information\n";
echo "========================\n\n";

echo "PHP Version: " . phpversion() . "\n";
echo "Server Time: " . date('Y-m-d H:i:s') . "\n";
echo "Script Path: " . $_SERVER['SCRIPT_FILENAME'] . "\n\n";
?>