<?php

namespace OpenHappen\Bot;

class Robots {

    protected $_request = NULL;

    public function __construct(string $shortURL) {
        $this->_request = new Request($shortURL . 'robots.txt');
    }

    public function retrieve() : array {
        /* Run request */
        list($status, $message) = $this->_request->retrieve();
        if (!$status) {
            return [FALSE, $message];
        }

        /* Success */
        return [TRUE, ''];
    }
}
