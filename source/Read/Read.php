<?php

declare(strict_types=1);

namespace OpenHappen\Read;

use OpenHappen\DataProvider;

/* Data providers */
require_once __DIR__ . '/../DataProvider/JSON.php.inc';

/* Bot scripts */
require_once __DIR__ . '/../Bot/Request.php.inc';

/* Check if you are running this script in CLI mode */
if (php_sapi_name() !== 'cli') {
    exit('Run only this script in CLI mode' . PHP_EOL);
}

/* Set some default info */
$urlValue = NULL;
$dataProviderValue = 'json';

/* Get url path */
$urlKey = array_search('--url', $argv);
if ($urlKey !== FALSE && !empty($argv[$urlKey + 1])) {
    $urlValue = $argv[$urlKey + 1];
}

/* Check if filename is empty */
if ($urlValue === NULL) {
    exit('No URL given');
}

/* Get data provider */
$dataProviderKey = array_search('--data-provider', $argv);
if ($dataProviderKey !== FALSE && !empty($argv[$dataProviderKey + 1])) {
    $dataProviderValue = $argv[$dataProviderKey + 1];
}

/* Get data provider object */
switch ($dataProviderValue) {
    case 'json':
        $dataProviderObj = new DataProvider\JSON;
        break;
    default:
        exit('Unspported data provider' . PHP_EOL);
}

/* Try to get raw content */
$result = $dataProviderObj->getRawContent($urlValue);
print_r($result);
