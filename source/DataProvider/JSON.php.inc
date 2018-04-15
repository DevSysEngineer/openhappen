<?php

namespace OpenHappen\DataProvider;

use OpenHappen\Bot;

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

    protected function _getPageDir(string $filename = NULL) : string {
        /* Create dir string */
        $dir = $this->_dir . DIRECTORY_SEPARATOR . 'Pages' . DIRECTORY_SEPARATOR;
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
        $dirWebsite = $this->_getPageDir();
        if (!file_exists($dirWebsite)) {
            if (!mkdir($dirWebsite, 0755, TRUE)) {
                return[FALSE, 'Failed to create website dir'];
            }
        }

        /* Success */
        return [TRUE, ''];
    }

    public function addPage(Bot\Page $page) : array {
        /* Create main object */
        $object = new \stdClass;
        $object->lastExport = time();

        /* Get request*/
        $request = $page->getRequest();

        /* Create page object */
        $pageObj = new \stdClass;

        /* Set hashmark hrefs */
        $pageObj->hashmarkHrefs = [];
        foreach ($page->getHashmarkHrefs() as $hashmarkHref) {
            $pageObj->hashmarkHrefs[] = $hashmarkHref->toStdClass($request->getSimpleURL());
        }

        /* Set internal hrefs */
        $pageObj->internalHrefs = [];
        foreach ($page->getInternalHrefs() as $internalHref) {
            $pageObj->internalHrefs[] = $internalHref->toStdClass($request->getSimpleURL());
        }

        /* Set external hrefs */
        $pageObj->externalHrefs = [];
        foreach ($page->getExternalHrefs() as $externalHref) {
            $pageObj->externalHrefs[] = $externalHref->toStdClass($request->getSimpleURL());
        }

        /* Add page to main object */
        $object->page = $pageObj;

        /* Success */
        return $this->_writeToFile($this->_getPageDir($request->getId()), $object);
    }

    public function retrievePage(string $url) : bool {
        return !file_exists($this->_getPageDir(Bot\Request::Base64URLEncode($url)));
    }
}
