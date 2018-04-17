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

    protected function _getPageDir(string $filename = NULL) : string {
        /* Create dir string */
        $dir = $this->_dir . DIRECTORY_SEPARATOR . 'Pages' . DIRECTORY_SEPARATOR;
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

    public function init() {
        /* Create page dir */
        $dirPage= $this->_getPageDir();
        if (!file_exists($dirPage)) {
            if (!mkdir($dirPage, 0755, TRUE)) {
                return[FALSE, 'Failed to create page dir'];
            }
        }

        /* Success */
        return [TRUE, ''];
    }

    public function addPage(Bot\Page $page) : array {
        /* Get request*/
        $request = $page->getRequest();
        $domainURL = $request->getDomainURL();

        /* Create main object */
        $object = new \stdClass;
        $object->version = $this->_version;
        $object->lastExport = time();
        $object->url = $request->getURL();

        /* Create page object */
        $pageObj = new \stdClass;
        $pageObj->lastModified = 0;
        $pageObj->title = $page->getTitle();
        $pageObj->headers = $page->getHeaders();
        $pageObj->paragraphs = $page->getParagraphs();

        /* Set hashmark hrefs */
        $pageObj->hashmarkHrefs = [];
        foreach ($page->getHashmarkHrefs() as $hashmarkHref) {
            $pageObj->hashmarkHrefs[] = $hashmarkHref->getURL($domainURL);
        }

        /* Set internal hrefs */
        $pageObj->internalHrefs = [];
        foreach ($page->getInternalHrefs() as $internalHref) {
            $pageObj->internalHrefs[] = $internalHref->getURL($domainURL);
        }

        /* Set external hrefs */
        $pageObj->externalHrefs = [];
        foreach ($page->getExternalHrefs() as $externalHref) {
            $pageObj->externalHrefs[] = $externalHref->getURL($domainURL);
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
