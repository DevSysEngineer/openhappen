<?php

declare(strict_types=1);

namespace OpenHappen\Bot;

class SmartDOM {

    const TYPE_PAGE = 'page';
    const TYPE_SITEMAP = 'sitemap';

    protected $_type = NULL;
    protected $_domDocument = NULL;

    public function __construct($type = self::TYPE_PAGE) {
        $this->_type = $type;
        $this->_domDocument = new \DOMDocument;
    }

    public static function ClearString(String $string) : string {
        return trim(preg_replace('/\s\s+/', ' ', str_replace(PHP_EOL, '', $string)));
    }

    public function load(string $content) : array {
        /* Load content by type */
        switch ($this->_type) {
            case self::TYPE_PAGE:
                /* Check if first char are html */
                if (substr($content, 0, 5) !== '<html') {
                    return [FALSE, 'Failed to load HTML'];
                }

                /* Load HTML */
                $status = @$this->_domDocument->loadHTML($content);
                if (!$status) {
                    return [FALSE, 'Failed to load HTML'];
                }
                break;
            case self::TYPE_SITEMAP:
                /* Check if first char are html */
                if (substr($content, 0, 5) !== '<?xml') {
                    return [FALSE, 'Failed to load XML'];
                }

                /* Load XML */
                $status = @$this->_domDocument->loadXML($content);
                if (!$status) {
                    return [FALSE, 'Failed to load XML'];
                }
                break;
            default:
                /* Failed */
                return [FALSE, 'Failed to read content because type is unknown'];
        }

        /* Success */
        return [TRUE, ''];
    }

    public function extractText(\DOMNode $node, array &$text) {
        if (in_array($node->nodeType, [XML_ELEMENT_NODE, XML_DOCUMENT_NODE, XML_DOCUMENT_FRAG_NODE])) {
            /* Get text and loop child nodes */
            foreach ($node->childNodes as $childNode) {
                $this->extractText($childNode, $text);
            }
        } elseif (in_array($node->nodeType, [XML_TEXT_NODE, XML_CDATA_SECTION_NODE])) {
            $clearText = self::ClearString($node->nodeValue);
            if (!empty($clearText)) {
                $text[] = $clearText;
            }
        }
    }

    public function getSitemapHrefs(array $filters = []) : array {
        /* Check if filter is active */
        $activeFilters = !empty($filters);

        /* Create default values */
        $sitemapHrefs = [];
        $paths = [];

        /* Get sitemap hrefs */
        $elements = $this->_domDocument->getElementsByTagName('sitemap');
        foreach ($elements as $element) {
            /* Get array of loc */
            $argsLoc = [];
            $childElements = $element->getElementsByTagName('loc');
            foreach ($childElements as $childElement) {
                $this->extractText($childElement, $argsLoc);
            }

            /* Check if raw href is not empty and not already exists */
            $rawLoc = implode($argsLoc, '');
            $empty = empty($rawLoc);
            if ($empty || (!$empty && in_array($rawLoc, $paths))) {
                continue;
            }

            /* Get array of last modified */
            $argsLastModified = [];
            $childElements = $element->getElementsByTagName('lastmod');
            foreach ($childElements as $childElement) {
                $this->extractText($childElement, $argsLastModified);
            }

            /* Convert array to string */
            $lastModified = empty($argsLastModified) ? NULL : implode($argsLastModified, '');

            /* Create Href  */
            $hrefObj = new Href($rawLoc, $lastModified);

            /* Get type and check if type is valid or exists in filter */
            $type = $hrefObj->getType();
            if ($type !== Request::TYPE_UNKNOWN &&
                (!$activeFilters || ($activeFilters && in_array($type, $filters)))) {
                $sitemapHrefs[] = $hrefObj;
                $paths[] = $rawLoc;
            }
        }

        /* Return sitemaps */
        return $sitemapHrefs;
    }

    public function getHrefs(array $filters = []) : array {
        /* Check if filter is active */
        $activeFilters = !empty($filters);

        /* Create default values */
        $hrefs = [];
        $paths = [];

        /* Get elements */
        $elements = [];
        switch ($this->_type) {
            case self::TYPE_PAGE:
                $elements = $this->_domDocument->getElementsByTagName('a');
                break;
            case self::TYPE_SITEMAP:
                $elements = $this->_domDocument->getElementsByTagName('url');
                break;
        }

        /* Get hrefs */
        foreach ($elements as $element) {
            /* Create some default values */
            $lastModified = NULL;
            $changeFreq = NULL;

            /* Get some info */
            switch ($this->_type) {
                case self::TYPE_SITEMAP:
                    /* Get array of hrefs */
                    $argsHref = [];
                    $childElements = $element->getElementsByTagName('loc');
                    foreach ($childElements as $childElement) {
                        $this->extractText($childElement, $argsHref);
                    }

                    /* Check if raw href is not empty and not already exists */
                    $rawHref = implode($argsHref, '');
                    $empty = empty($rawHref);
                    if ($empty || (!$empty && in_array($rawHref, $paths))) {
                        continue 2;
                    }

                    /* Get array of last modified */
                    $argsLastModified = [];
                    $childElements = $element->getElementsByTagName('lastmod');
                    foreach ($childElements as $childElement) {
                        $this->extractText($childElement, $argsLastModified);
                    }

                    /* Get change fred */
                    $argsChangeFreq = [];
                    $childElements = $element->getElementsByTagName('changefreq');
                    foreach ($childElements as $childElement) {
                        $this->extractText($childElement, $argsLastModified);
                    }

                    /* Convert array to string */
                    $lastModified = empty($argsLastModified) ? NULL : implode($argsLastModified, '');
                    $changeFreq = empty($argsChangeFreq) ? NULL : implode($argsChangeFreq, '');
                    break;
                case self::TYPE_PAGE:
                    $rawHref = $element->getAttribute('href');
                    $empty = empty($rawHref);
                    if ($empty || (!$empty && in_array($rawHref, $paths))) {
                        continue 2;
                    }
                    break;
                default:
                    continue 2;
            }

            /* Create Href  */
            $hrefObj = new Href($rawHref, $lastModified, $changeFreq);

            /* Get type and check if type is valid or exists in filter */
            $type = $hrefObj->getType();
            if ($type !== Request::TYPE_UNKNOWN &&
                (!$activeFilters || ($activeFilters && in_array($type, $filters)))) {
                $hrefs[] = $hrefObj;
                $paths[] = $rawHref;
            }
        }

        /* Return hrefs */
        return $hrefs;
    }

    public function getTitle() {
        /* Create empty string */
        $titles = [];

        /* Get elements */
        $elements = $this->_domDocument->getElementsByTagName('title');
        foreach ($elements as $element) {
            $this->extractText($element, $titles);
        }

        /* Convert array to string */
        return implode(' ', $titles);
    }

    public function getHeaders() : array {
        /* Get all headers */
        $headers = [];
        $tagNames = ['h1', 'h2', 'h3', 'h4', 'h5', 'h5', 'header'];
        foreach ($tagNames as $tagName) {
            $elements = $this->_domDocument->getElementsByTagName($tagName);
            foreach ($elements as $element) {
                $header = [];
                $this->extractText($element, $header);
                if (!empty($header)) {
                    $headers[] = implode(' ', $header);
                }
            }
        }

        /* Return headers */
        return $headers;
    }

    public function getParagraphs() : array {
        /* Get paragraphs */
        $paragraphs = [];
        $elements = $this->_domDocument->getElementsByTagName('p');
        foreach ($elements as $element) {
            $paragraph = [];
            $this->extractText($element, $paragraph);
            if (!empty($paragraph)) {
                $paragraphs[] = implode(' ', $paragraph);
            }
        }

        /* Return paragraphs */
        return $paragraphs;
    }
}
