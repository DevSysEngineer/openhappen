<?php

declare(strict_types=1);

namespace OpenHappen\Bot;

class Location {

    protected $_request = NULL;
    protected $_robots = NULL;
    protected $_smartDom = NULL;

    protected $_logs = [];

    protected $_title = '';
    protected $_headers = [];
    protected $_paragraphs = [];

    public function __construct(string $url, Robots $robots = NULL) {
        $this->_request = new Request($url);
        $this->_robots = ($robots === NULL) ? new Robots($this->_request->getDomainURL()) : $robots;
    }

    public function init() : array {
        /* Only run request if page is not retrieved */
        if (!$this->_robots->isRetrieved()) {
            list($status, $message) = $this->_robots->retrieve();
            if (!$status) {
                $this->_logs[] = 'Failed to retrieve robots.txt: ' . $message;
            }
        }

        /* Success */
        return [TRUE, ''];
    }

    public function retrieve() : array {
        /* Check if path is allowed */
        $path = $this->_request->getPath();
        if (!$this->_robots->isPathAllowed($path)) {
            return [FALSE, 'Blocked by robots.txt'];
        }

        /* Success */
        return [TRUE, ''];
    }

    public function getLogs() {
        return $this->_logs;
    }

    public function getRequest() {
        return $this->_request;
    }

    public function getRobots() {
        return $this->_robots;
    }

    public function getTitle() {
        return $this->_title;
    }

    public function getHeaders() {
        return $this->_headers;
    }

    public function getParagraphs() {
        return $this->_paragraphs;
    }

}
