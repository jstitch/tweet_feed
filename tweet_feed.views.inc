<?php

/**
 * implementation of views_data()
 */
function tweet_feed_views_data() {
  return array(
    'tweet_feed' => array(
      'table' => array(
        'group' => 'Twitter Feed',
        'base' => array(
          'field' => 'tid',
          'title' => 'Twitter Feed Posts',
          'weight' => -10,
        ),
      ),
      'tid' => array(
        'title' => t('Tid'),
        'help' => t('The Drupal ID of the tweet'),
        'field' => array(
          'handler' => 'views_handler_field_numeric',
          'click sortable' => TRUE,
        ),
        'filter' => array(
          'handler' => 'views_handler_filter_numeric',
        ),
        'sort' => array(
          'handler' => 'views_handler_sort',
        ),
      ),
      'tweet' => array(
        'title' => t('Tweet Text'),
        'help' => t('The actual 140 character tweet text.'),
        'field' => array(
          'handler' => 'views_handler_tweet_field',
          'click sortable' => FALSE,
        ),
        'sort' => array(
          'handler' => 'views_handler_sort',
        ),
      ),
      'created_at' => array(
        'title' => t('Tweet Creation Time'),
        'help' => t('The creation time stamp of the tweet.'),
        'field' => array(
          'handler' => 'views_handler_field_date',
        ),
        'sort' => array(
          'handler' => 'views_handler_sort',
        ),
      ),
      'user_id' => array(
        'title' => t('User ID'),
        'help' => t('The user_id of the person making the tweet'),
        'field' => array(
          'handler' => 'views_handler_field_numeric',
          'click sortable' => TRUE,
        ),
        'filter' => array(
          'handler' => 'views_handler_filter_numeric',
        ),
        'sort' => array(
          'handler' => 'views_handler_sort',
        ),
      ),
      'profile_image_url' => array(
        'title' => t('Profile Image URL'),
        'help' => t('The url to the profile image of the tweeter.'),
        'field' => array(
          'handler' => 'views_handler_field',
        ),
      ),
      'screen_name' => array(
        'title' => t('Screen Name'),
        'help' => t('The screen name of the tweeter.'),
        'field' => array(
          'handler' => 'views_handler_field',
        ),
      ),
      'hashtags' => array(
        'title' => t('Hashtags'),
        'help' => t('The Hashtags associated with a tweet.'),
        'field' => array(
          'handler' => 'views_handler_field',
        ),
        'argument' => array(
          'handler' => 'views_handler_argument_hashtag',
        ),
      ),
    ),
  );
}

/**
 * override views_handler_field::render so that it does not sanitize our URL's and other links
 */
class views_handler_tweet_field extends views_handler_field {

  function render($values) {
    $tweet = $this->get_value($values);

    /* based on our preference, assign all links to new windows or to the same window */
    $target = (variable_get('tweet_feed_new_window',0) == 1) ? '_blank' : '_self';

    /* look for twitter handles and make them clickable */
    /* modified so that slashes in the twitter handle are counted */
    $pattern = '/@([A-Za-z0-9_]{1,15})(?![.A-Za-z])/';
    $replace  = '<a target="'.$target.'" href="http://twitter.com/'.strtolower('\1').'">@\1</a>';
    $tweet   = preg_replace($pattern,$replace,$tweet);

    /* look for twitter hash tags and make them clickable */
    /* modified so that slashes in the twitter handle are counted */
    $tweet = preg_replace('/(^|\s)#(\w*+\w*)/', '\1<a target="'.$target.'" href="http://twitter.com/search?q=%23\2">#\2</a>', $tweet);
    
    $tweet = utf8_decode($tweet);
    return htmlspecialchars_decode($tweet);
  }
  
}


class views_handler_argument_hashtag extends views_handler_argument_string {

  function query($group_by = FALSE) {
    $argument = '%' . $this->argument . '%';
    if (!empty($this->options['transform_dash'])) {
      $argument = strtr($argument, '-', ' ');
    }

    if (!empty($this->options['break_phrase'])) {
      views_break_phrase_string($argument, $this);
    }
    else {
      $this->value = array($argument);
      $this->operator = 'or';
    }

    if (!empty($this->definition['many to one'])) {
      if (!empty($this->options['glossary'])) {
        $this->helper->formula = TRUE;
      }
      $this->helper->ensure_my_table();
      $this->helper->add_filter();
      return;
    }

    $this->ensure_my_table();
    $formula = FALSE;
    if (empty($this->options['glossary'])) {
      $field = "$this->table_alias.$this->real_field";
    }
    else {
      $formula = TRUE;
      $field = $this->get_formula();
    }

    if (count($this->value) > 1) {
      $operator = 'IN';
      $argument = $this->value;
    }
    else {
      $operator = 'LIKE';
    }

    if ($formula) {
      $placeholder = $this->placeholder();
      if ($operator == 'IN') {
        $field .= " IN($placeholder)";
      }
      else {
        $field .= ' = ' . $placeholder;
      }
      $placeholders = array(
        $placeholder => $argument,
      );
      $this->query->add_where_expression(0, $field, $placeholders);
    }
    else {
      $this->query->add_where(0, $field, $argument, $operator);
    }
  }
}