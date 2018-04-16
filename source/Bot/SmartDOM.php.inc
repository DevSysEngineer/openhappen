<?php

namespace OpenHappen\Bot;

class SmartDOM {

    const TYPE_PAGE = 'page';
    const TYPE_SITEMAP = 'sitemap';

    protected $_type = NULL;
    protected $_domDocument = NULL;

    public function __construct($type = TYPE_PAGE) {
        $this->_type = $type;
        $this->_domDocument = new \DOMDocument;
    }

    public function load(string $content) {
        /* Load content by type */
        switch ($this->_type) {
            case self::TYPE_PAGE:
                $status = @$this->_domDocument->loadHTML($content);
                if (!$status) {
                    return [FALSE, 'Failed to load HTML'];
                }
                break;
            default:
                /* Failed */
                return [FALSE, 'Failed to read content because type is unknown'];
        }

        /* Success */
        return [TRUE, ''];
    }

    public function getHrefs($filters = []) {
        /* Check if filter is active */
        $activeFilters = !empty($filters);

        /* Create default values */
        $hrefs = [];
        $paths = [];

        /* Get hrefs */
        $elements = $this->_domDocument->getElementsByTagName('a');
        foreach ($elements as $element) {
            $rawHref = $element->getAttribute('href');
            if (!empty($rawHref) && !in_array($rawHref, $paths)) {
                /* Create Href  */
                $hrefObj = new Href($rawHref);

                /* Get type and check if type is valid or exists in filter */
                $type = $hrefObj->getType();
                if ($type !== Request::TYPE_UNKNOWN &&
                    (!$activeFilters || ($activeFilters && in_array($type, $filters)))) {
                    $hrefs[] = $hrefObj;
                    $paths[] = $rawHref;
                }
            }
        }

        /* Return hrefs */
        return $hrefs;
    }
}
