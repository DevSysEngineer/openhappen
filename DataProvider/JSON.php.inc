<?php

namespace DataProvider;

use Bot;

class JSON {

    protected $_dir = NULL;

    public function __construct(\stdClass $config = NULL) {
        /* Check if config dir not exists  */
        if (empty($config) || empty($config->dir)) {
            $this->_dir = __DIR__ . DIRECTORY_SEPARATOR . '..' . DIRECTORY_SEPARATOR . 'Data';
        } else {
            $this->_dir = $config->dir;
        }
    }

    protected function _getWebsiteDir(string $filename = NULL) : string {
        /* Create dir string */
        $dir = $this->_dir . DIRECTORY_SEPARATOR . 'Websites' . DIRECTORY_SEPARATOR;
        if ($filename !== NULL) {
            $dir .= $filename . '.json';
        }

        /* Return dir */
        return $dir;
    }

    protected function _writeToFile(string $path, \stdClass $stdClass) : array {
        /* Write file */
        if (!file_put_contents($path, json_encode($stdClass), LOCK_EX)) {
            return[FALSE, 'Failed to write file'];
        }

        /* Success */
        return [TRUE, ''];
    }

    public function init() {
        /* Create website dir */
        $dirWebsite = $this->_getWebsiteDir();
        if (!file_exists($dirWebsite)) {
            if (!mkdir($dirWebsite, 0755, TRUE)) {
                return[FALSE, 'Failed to create website dir'];
            }
        }

        /* Success */
        return [TRUE, ''];
    }

    public function addWebsite(Bot\Website $website) : array {
        /* Create main object */
        $object = new \stdClass;
        $object->lastExport = time();

        /* Get request*/
        $request = $website->getRequest();

        /* Create website object */
        $websiteObj = new \stdClass;

        /* Set hashmark hrefs */
        $websiteObj->hashmarkHrefs = [];
        foreach ($website->getHashmarkHrefs() as $hashmarkHref) {
            $websiteObj->hashmarkHrefs[] = $hashmarkHref->toStdClass($request->getSimpleURL());
        }

        /* Set internal hrefs */
        $websiteObj->internalHrefs = [];
        foreach ($website->getInternalHrefs() as $internalHref) {
            $websiteObj->internalHrefs[] = $internalHref->toStdClass($request->getSimpleURL());
        }

        /* Set external hrefs */
        $websiteObj->externalHrefs = [];
        foreach ($website->getExternalHrefs() as $externalHref) {
            $websiteObj->externalHrefs[] = $externalHref->toStdClass($request->getSimpleURL());
        }

        /* Add website to main object */
        $object->website = $websiteObj;

        /* Success */
        return $this->_writeToFile($this->_getWebsiteDir($request->getId()), $object);
    }

    public function retrieveWebsite(string $url) : bool {
        return !file_exists($this->_getWebsiteDir(Bot\Request::Base64URLEncode($url)));
    }

    public function getWebsite(string $url) {

    }
}
