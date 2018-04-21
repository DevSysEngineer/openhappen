<?php

declare(strict_types=1);

namespace OpenHappen\Bot;

class Robots {

    const USER_AGENT_ALL = '*';
    const USER_AGENT_OPENHAPPEN_BOT = 'openhappenbot';

    const DIRECTIVE_ALLOW = 'allow';
    const DIRECTIVE_DISALLOW = 'disallow';
    const DIRECTIVE_SITEMAP = 'sitemap';
    const DIRECTIVE_USER_AGENT = 'user-agent';

    protected $_isRetrieved = FALSE;

    protected $_request = NULL;

    protected $_crawlDelay = 10;
    protected $_sitemapHrefs = [];

    protected $_botFound = FALSE;
    protected $_botAllows = [];
    protected $_botDisallows = [];

    protected $_allAllows = [];
    protected $_allDisallows = [];

    public function __construct(string $shortURL) {
        $this->_request = new Request($shortURL . 'robots.txt');
    }

    public function getAllowedUserAgents() {
        return [self::USER_AGENT_ALL, self::USER_AGENT_OPENHAPPEN_BOT];
    }

    public function getAllowedDirectives() {
        return [self::DIRECTIVE_SITEMAP, self::DIRECTIVE_USER_AGENT, self::DIRECTIVE_ALLOW, self::DIRECTIVE_DISALLOW];
    }

    public function retrieve() : array {
        /* Check if request is already retrieved */
        if ($this->_isRetrieved) {
            return [TRUE, ''];
        }

        /* Run request */
        list($status, $message) = $this->_request->retrieve();
        if (!$status) {
            return [FALSE, $message];
        }

        /* Check if content is empty */
        list($content, $message) = $this->_request->getContent();
        if ($content === NULL) {
            return [FALSE, $message];
        }

        /* Set some data */
        $activeUserAgent = NULL;

        /* Extract content to lines and loop each line */
        $lines = explode(PHP_EOL, $content);
        foreach ($lines as $line) {
            /* Check if line is valid format */
            $line = preg_replace('/#.*/', '', $line);
            $parts = explode(':', $line, 2);
            if (count($parts) < 2) {
                continue;
            }

            /* Get directive and value from parts */
            $directive = strtolower(trim($parts[0]));
            $value = trim($parts[1]);

            /* Check if directive is not supported and value is not empty */
            if (!in_array($directive, $this->getAllowedDirectives()) || empty($value)) {
                continue;
            }

            /* Switch case for special directive that is not linked to group */
            switch ($directive) {
                case self::DIRECTIVE_SITEMAP:
                    $this->_sitemapHrefs[] = new Href($value, NULL, Href::CHANGE_FREY_ALWAYS);
                    continue;
            }

            /* Check if directive is useragent */
            if ($directive === self::DIRECTIVE_USER_AGENT) {
                /* Check if value is allowed */
                if (in_array($value, $this->getAllowedUserAgents())) {
                    /* Set active user agent */
                    $activeUserAgent = $value;
                    if ($activeUserAgent === self::USER_AGENT_OPENHAPPEN_BOT) {
                        $this->_botFound = TRUE;
                    }
                } else {
                    /* Reset active user agent */
                    $activeUserAgent = NULL;
                }

                /* Go to next line */
                continue;
            } elseif ($activeUserAgent === NULL) {
                /* Go to next line */
                continue;
            }

            /* Switch case for linking directive to group */
            switch ($activeUserAgent) {
                case self::USER_AGENT_OPENHAPPEN_BOT:
                    $prefix = '_bot';
                    break;
                case self::USER_AGENT_ALL:
                    $prefix = '_all';
                    break;
                default:
                    $prefix = NULL;
            }

            /* Set value */
            if ($prefix !== NULL) {
                switch ($directive) {
                    case self::DIRECTIVE_ALLOW:
                        $this->{$prefix . 'Allows'}[] = $value;
                        break;
                    case self::DIRECTIVE_DISALLOW:
                        $this->{$prefix . 'Disallows'}[] = $value;
                        break;
                }
            }
        }

        /* Data is retrieved */
        $this->_isRetrieved = TRUE;

        /* Success */
        return [TRUE, ''];
    }

    public function isPathAllowed($parentPath) {
        /* Get prefix */
        $prefix = $this->_botFound ? '_bot' : '_all';

        /* Check if path is not allowed */
        $foundDisallow = FALSE;
        foreach ($this->{$prefix . 'Disallows'} as $disallowPath) {
            if (substr($parentPath, 0, strlen($disallowPath)) === $disallowPath) {
                $foundDisallow = TRUE;
                break;
            }
        }

        /* Path is not allowed, check if there is any key for this path  */
        if ($foundDisallow) {
            foreach ($this->{$prefix . 'Allows'} as $allowPath) {
                if (substr($parentPath, 0, strlen($allowPath)) === $allowPath) {
                    return TRUE;
                }
            }

            /* Not allowed */
            return FALSE;
        }

        /* Allowed */
        return TRUE;
    }

    public function isRetrieved() {
        return $this->_isRetrieved;
    }

    public function getCrawlDelay() {
        return $this->_crawlDelay;
    }

    public function getSitemapHrefs() {
        return $this->_sitemapHrefs;
    }
}
