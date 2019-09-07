<?php

namespace OpenHappen\DataProvider;

use OpenHappen\Bot;

class JSON {

    const MINIMAL_LENGTH = 4;

    protected $_version = 'v0.0.1';
    protected $_dir = NULL;
    protected $_timezone = NULL;

    public function __construct(\stdClass $config = NULL) {
        /* Check if config dir not exists  */
        if (empty($config) || empty($config->dir)) {
            $this->_dir = implode(DIRECTORY_SEPARATOR, [__DIR__, '..', '..', 'assets', 'Data']);
        } else {
            $this->_dir = $config->dir;
        }

        /* Set UTC timezone */
        $this->_timezone = new \DateTimeZone('UTC');
    }

    protected function _log(string $text) {
        echo '[' . date('r') . '][JSON] ' . $text . PHP_EOL;
    }

    protected function _getLocationDir(string $filename = NULL) : string {
        /* Create dir string */
        $dir = $this->_dir . DIRECTORY_SEPARATOR . 'Location' . DIRECTORY_SEPARATOR;
        if ($filename !== NULL) {
            $dir .= $filename;
        }

        /* Return dir */
        return $dir;
    }

    protected function _writeToFile(string $path, \stdClass $stdClass) : array {
        /* Write file */
        if (!file_put_contents($path, gzcompress(json_encode($stdClass), 9), LOCK_EX)) {
            return[FALSE, 'Failed to write file'];
        }

        /* Success */
        return [TRUE, ''];
    }

    protected function _getContent(string $path) : array {
        /* Get raw content */
        $rawContent = @file_get_contents($path);
        if ($rawContent === FALSE) {
            return [FALSE, 'File not exists'];
        }

        /* Convert raw content to readable content */
        $uncompressContent = @gzuncompress($rawContent);
        if ($uncompressContent === FALSE) {
            return [FALSE, 'Failed to uncompress content'];
        }

        /* Convert uncompress content to object */
        $object = @json_decode($uncompressContent);
        if ($object === FALSE) {
            return [FALSE, 'Failed to decode content'];
        }

        /* Return object */
        return [$object, ''];
    }

    protected function _simplifyPart(string $word, array $parts, array &$newLine, array &$allWords) : bool {
        /* Create some default values */
        $foundWord = FALSE;
        $customWord = [];

        /* Loop parts */
        foreach ($parts as $partValue) {
            if (count($customWord) >= self::MINIMAL_LENGTH) {
                $newWord = implode($customWord, '');
                $foundWord = array_search($newWord, $allWords);
                if ($foundWord !== FALSE) {
                    /* Replace part of a string. */
                    $newLine[] = str_replace($newWord, '[' . $foundWord . '%', $word);

                    /* Create some log */
                    $this->_log('Found part in list: ' . $word . ' (' . $newWord . '), key: ' . $foundWord);
                    break;
                } else {
                    $customWord[] = $partValue;
                }
            } else {
                $customWord[] = $partValue;
            }
        }

        /* Return result */
        return $foundWord;
    }

    protected function _simplifyString(string $string, array &$allWords) {
        $newLine = [];
        $words = explode(' ', $string);
        foreach ($words as $word) {
            if (strlen($word) >= self::MINIMAL_LENGTH) {
                $searchKey = array_search($word, $allWords);
                if ($searchKey === FALSE) {
                    /* Set default state on FALSE */
                    $foundWord = FALSE;

                    /* Try to search parts of a word in all words */
                    $strArgs = str_split($word);
                    $foundWord = $this->_simplifyPart($word, $strArgs, $newLine, $allWords);

                     /* Try to search parts of a word in all words in reverse way */
                    if ($foundWord === FALSE) {
                        $foundWord = $this->_simplifyPart($word, array_reverse($strArgs), $newLine, $allWords);
                    }

                    /* No part found, create new word */
                    if ($foundWord === FALSE) {
                        /* Set new word */
                        $allWords[] = $word;

                        /* Set new line */
                        $index = count($allWords);
                        $newLine[] = '[' . ($index - 1) . '%';
                    }
                } else {
                    $newLine[] = '[' . $searchKey . '%';
                }
            } else {
                $newLine[] = $word;
            }
        }

        /* Return line */
        return str_replace(['% [', ' [', '% ', '%. ', '. ', ', '], ['&', '~', '^', '`', '.', ','], implode($newLine, ' '));
    }

    protected function _simplifyArray(array $lines, &$allWords) {
        /* Convert lines */
        $newLines = [];
        foreach ($lines as $key => $line) {
            $newLines[$key] = $this->_simplifyString($line, $allWords);
        }

        /* Return new lines */
        return $newLines;
    }

    public function init() {
        /* Create page dir */
        $dirLocation = $this->_getLocationDir();
        if (!file_exists($dirLocation)) {
            if (!mkdir($dirLocation, 0755, TRUE)) {
                return[FALSE, 'Failed to create location dir'];
            }
        }

        /* Success */
        return [TRUE, ''];
    }

    public function addLocation(Bot\Location $location, Bot\Href $href) : array {
        /* Get request*/
        $request = $location->getRequest();

        /* Create variable for all words */
        $allWords = [];

        /* Create date object */
        $dateTime = new \DateTime('now', $this->_timezone);

        /* Create main object */
        $object = new \stdClass;
        $object->version = $this->_version;
        $object->lastExport = $dateTime->getTimestamp();
        $object->nextExport = $href->getNextExport();
        $object->changeFrey = $href->getChangeFreq();
        $object->url = $request->getURL();

        /* Create page object */
        $locationObj = new \stdClass;
        $locationObj->title = $this->_simplifyString($location->getTitle(), $allWords);
        $locationObj->headers = $this->_simplifyArray($location->getHeaders(), $allWords);
        $locationObj->paragraphs = $this->_simplifyArray($location->getParagraphs(), $allWords);
        $locationObj->words = $allWords;

        /* Add page to main object */
        $object->location = $locationObj;

        /* Success */
        return $this->_writeToFile($this->_getLocationDir($request->getId()), $object);
    }

    public function changeLocationValues(string $url, array $args) : array {
        /* Get content */
        $path = $this->_getLocationDir(Bot\Request::Base64URLEncode($url));
        list($object, $message) = $this->_getContent($path);
        if ($object === NULL) {
            return [FALSE, $message];
        }

        /* Set values */
        foreach ($args as $key => $value) {
            $object->{$key} = $value;
        }

        /* Success */
        return $this->_writeToFile($path, $object);
    }

    public function changeLocationValue(string $url, string $key, $value) : array {
        /* Get content */
        $path = $this->_getLocationDir(Bot\Request::Base64URLEncode($url));
        list($object, $message) = $this->_getContent($path);
        if ($object === NULL) {
            return [FALSE, $message];
        }

        /* Change data */
        $object->{$key} = $value;

        /* Success */
        return $this->_writeToFile($path, $object);
    }

    public function retrieveLocation(string $url) : bool {
        /* Check if location not exists */
        if (!$this->existsLocation($url)) {
            return TRUE;
        }

        /* Get content */
        $path = $this->_getLocationDir(Bot\Request::Base64URLEncode($url));
        list($object, $message) = $this->_getContent($path);
        if ($object === NULL) {
            return [FALSE, $message];
        }

        /* Check if location always must be checked */
        if ($object->changeFrey === Bot\Href::CHANGE_FREY_ALWAYS) {
            return TRUE;
        }

        /* Create date object */
        $dateTime = new \DateTime('now', $this->_timezone);
        $currentTimestamp = $dateTime->getTimestamp();
        $dateTime->setTimestamp($object->nextExport);
        $nextTmestamp = $dateTime->getTimestamp();

        /* Check if last export time with modified data is not newer than current time */
        return ($nextTmestamp < $currentTimestamp);
    }

    public function getRawContent(string $url) : \stdClass {
         /* Check if location not exists */
        if (!$this->existsLocation($url)) {
            $object = new \stdClass;
            $object->error = 'No data found for this URL';
            return $object;
        }

        /* Get content */
        $path = $this->_getLocationDir(Bot\Request::Base64URLEncode($url));
        list($object, $message) = $this->_getContent($path);
        if ($object === NULL) {
            $object = new \stdClass;
            $object->error = $message;
            return $object;
        }

        /* Translate title */
        $object->title = $object->title;

        /* Return object */
        return $object;
    }

    public function existsLocation(string $url) : bool {
        return file_exists($this->_getLocationDir(Bot\Request::Base64URLEncode($url)));
    }
}
