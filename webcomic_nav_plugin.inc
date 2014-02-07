<?php

/**
 * @file
 * Definition of views_plugin_pager_full.
 */

/**
 * The plugin to handle full pager.
 *
 * @ingroup views_pager_plugins
 */
class webcomic_nav_plugin extends views_plugin_pager {
  function summary_title() {
    return t('Show All');
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['items_per_page'] = array('default' => 1);
    $options['offset'] = array('default' => 0);
    $options['id'] = array('default' => 0);
    $options['total_pages'] = array('default' => '');
    $options['quantity'] = array('default' => 0);
    $options['expose'] = array(
      'contains' => array(
        'items_per_page' => array('default' => FALSE, 'bool' => TRUE),
        'items_per_page_label' => array('default' => 'Items per page', 'translatable' => TRUE),
        'items_per_page_options' => array('default' => '5, 10, 20, 40, 60'),
        'items_per_page_options_all' => array('default' => FALSE, 'bool' => TRUE),
        'items_per_page_options_all_label' => array('default' => '- All -', 'translatable' => TRUE),

        'offset' => array('default' => FALSE, 'bool' => TRUE),
        'offset_label' => array('default' => 'Offset', 'translatable' => TRUE),
      ),
    );
    $options['tags'] = array(
      'contains' => array(
        'first' => array('default' => '<< First', 'translatable' => TRUE),
        'previous' => array('default' => '< Previous', 'translatable' => TRUE),
        'next' => array('default' => 'Next >', 'translatable' => TRUE),
        'last' => array('default' => 'Latest >>', 'translatable' => TRUE),
      ),
    );
    return $options;
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $pager_text = $this->display->handler->get_pager_text();

    $form['tags'] = array (
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#tree' => TRUE,
      '#title' => t('Tags'),
      '#input' => TRUE,
      '#description' => t('A lists of labels for the controls in the pager'),
    );

    $form['tags']['first'] = array(
      '#type' => 'textfield',
      '#title' => t('Text for "first"-link'),
      '#description' => t('Text for "first"-link'),
      '#default_value' => $this->options['tags']['first'],
    );

    $form['tags']['previous'] = array(
      '#type' => 'textfield',
      '#title' => t('Text for "previous"-link'),
      '#description' => t('Text for "previous"-link'),
      '#default_value' => $this->options['tags']['previous'],
    );

    $form['tags']['next'] = array(
      '#type' => 'textfield',
      '#title' => t('Text for "next"-link'),
      '#description' => t('Text for "next"-link'),
      '#default_value' => $this->options['tags']['next'],
    );

    $form['tags']['last'] = array(
      '#type' => 'textfield',
      '#title' => t('Text for "last"-link'),
      '#description' => t('Text for "last"-link'),
      '#default_value' => $this->options['tags']['last'],
    );
  }

  function query() {
    if ($this->items_per_page_exposed()) {
      if (!empty($_GET['items_per_page']) && $_GET['items_per_page'] > 0) {
        $this->options['items_per_page'] = $_GET['items_per_page'];
      }
      elseif (!empty($_GET['items_per_page']) && $_GET['items_per_page'] == 'All' && $this->options['expose']['items_per_page_options_all']) {
        $this->options['items_per_page'] = 0;
      }
    }
    if ($this->offset_exposed()) {
      if (isset($_GET['offset']) && $_GET['offset'] >= 0) {
        $this->options['offset'] = $_GET['offset'];
      }
    }

    $limit = $this->options['items_per_page'];
    $offset = $this->current_page * $this->options['items_per_page'] + $this->options['offset'];
    if (!empty($this->options['total_pages'])) {
      if ($this->current_page >= $this->options['total_pages']) {
        $limit = $this->options['items_per_page'];
        $offset = $this->options['total_pages'] * $this->options['items_per_page'];
      }
    }

    $this->view->query->set_limit($limit);
    $this->view->query->set_offset($offset);
  }

  function render($input) {
    $pager_theme = views_theme_functions('pager', $this->view, $this->display);
    // The 0, 1, 3, 4 index are correct. See theme_pager documentation.
    $tags = array(
      4 => $this->options['tags']['first'],
      3 => $this->options['tags']['previous'],
      1 => $this->options['tags']['next'],
      0 => $this->options['tags']['last'],
    );
    $output = theme($pager_theme, array(
      'tags' => $tags,
      'element' => $this->options['id'],
      'parameters' => $input,
      'quantity' => $this->options['quantity'],
    ));
    return $output;
  }

  /**
   * Set the current page.
   *
   * @param $number
   *   If provided, the page number will be set to this. If NOT provided,
   *   the page number will be set from the global page array.
   */
  function set_current_page($number = NULL) {
    if (isset($number)) {
      $this->current_page = $number;
      return;
    }

    // If the current page number was not specified, extract it from the global
    // page array.
    global $pager_page_array;

    if (empty($pager_page_array)) {
      $pager_page_array = array();
    }

    // Fill in missing values in the global page array, in case the global page
    // array hasn't been initialized before.
    $page = isset($_GET['page']) ? explode(',', $_GET['page']) : array();

    for ($i = 0; $i <= $this->options['id'] || $i < count($pager_page_array); $i++) {
      $pager_page_array[$i] = empty($page[$i]) ? 0 : $page[$i];
    }

    $this->current_page = intval($pager_page_array[$this->options['id']]);

    if ($this->current_page < 0) {
      $this->current_page = 0;
    }
  }

  function get_pager_total() {
    if ($items_per_page = intval($this->get_items_per_page())) {
      return ceil($this->total_items / $items_per_page);
    }
    else {
      return 1;
    }
  }

  /**
   * Update global paging info.
   *
   * This is called after the count query has been run to set the total
   * items available and to update the current page if the requested
   * page is out of range.
   */
  function update_page_info() {
    if (!empty($this->options['total_pages'])) {
      if (($this->options['total_pages'] * $this->options['items_per_page']) < $this->total_items) {
        $this->total_items = $this->options['total_pages'] * $this->options['items_per_page'];
      }
    }

    // Don't set pager settings for items per page = 0.
    $items_per_page = $this->get_items_per_page();
    if (!empty($items_per_page)) {
      // Dump information about what we already know into the globals.
      global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;
      // Set the limit.
      $pager_limits[$this->options['id']] = $this->options['items_per_page'];
      // Set the item count for the pager.
      $pager_total_items[$this->options['id']] = $this->total_items;
      // Calculate and set the count of available pages.
      $pager_total[$this->options['id']] = $this->get_pager_total();

      // See if the requested page was within range:
      if ($this->current_page < 0) {
        $this->current_page = 0;
      }
      else if ($this->current_page >= $pager_total[$this->options['id']]) {
        // Pages are numbered from 0 so if there are 10 pages, the last page is 9.
        $this->current_page = $pager_total[$this->options['id']] - 1;
      }

      // Put this number in to guarantee that we do not generate notices when the pager
      // goes to look for it later.
      $pager_page_array[$this->options['id']] = $this->current_page;
    }
  }
}
