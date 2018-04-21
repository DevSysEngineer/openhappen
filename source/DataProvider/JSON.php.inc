<?php

namespace OpenHappen\DataProvider;

use OpenHappen\Bot;

class JSON {

    protected $_version = 'v0.0.1';
    protected $_dir = NULL;

    public function __construct(\stdClass $config = NULL) {
        /* Check if config dir not exists  */
        if (empty($config) || empty($config->dir)) {
            $this->_dir = implode(DIRECTORY_SEPARATOR, [__DIR__, '..', '..', 'assets', 'Data']);
        } else {
            $this->_dir = $config->dir;
        }
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
        return [ $object, '' ];
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
        $domainURL = $request->getDomainURL();

        /* Create main object */
        $object = new \stdClass;
        $object->version = $this->_version;
        $object->lastExport = time();
        $object->nextExport = 0;
        $object->changeFrey = $href->getChangeFreq();
        $object->url = $request->getURL();

        /* Create page object */
        $locationObj = new \stdClass;
        $locationObj->title = $location->getTitle();
        $locationObj->headers = $location->getHeaders();
        $locationObj->paragraphs = $location->getParagraphs();

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


        return FALSE;
    }

    public function existsLocation(string $url) : bool {
        return file_exists($this->_getLocationDir(Bot\Request::Base64URLEncode($url)));
    }
}
