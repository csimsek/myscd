<?php
for ($i = 0; $i < 100000; $i++) {
    $fd = fsockopen('image', 3301);
    echo 'connected' . PHP_EOL;
    fputs($fd, 'next');
    echo 'command sent' . PHP_EOL;
    $a = fgets($fd);
    echo $i . '-' . trim($a) . PHP_EOL;
    fclose($fd);
}
