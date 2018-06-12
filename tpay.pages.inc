<?php

/**
* page callback for user/%/tpay
* shows user subscription page
* @see tpay_menu
*/
function tpay_user_subscription_page($uid) {
  return drupal_get_form('tpay_user_subscription_form',$uid);
}

/**
* form shows table of user current subscriptions
*/
function tpay_user_subscription_form($form, &$form_state, $uid){
  $header = array(
    'contract_id' => t('Contract ID'),
    'product' => t('Product'),
    'next_payment_date' => t('Next Payment Date'),
    'status' => t('Status'),
    'msisdn' => t('Mobile Number'),
  );
  $query = db_select('tpay_contracts','tc')->extend('pagerDefault');
  $query->join('users','u','tc.customer_account_number = u.uid');
  $query->condition('tc.customer_account_number', $uid,'=')
  ->condition('tc.status','Active','=');
  $query->fields('tc',array('product_sku'))
  ->fields('tc',array('contract_id'))
  ->fields('tc',array('status'))
  ->fields('tc',array('msisdn'))
  ->fields('tc',array('next_payment_date'))
  ->orderBy('next_payment_date', 'DESC');
  $results = $query->execute();
  $rows = array();
  foreach ($results as $result ) {
    $rows[$result->contract_id]= array(
      'contract_id' => $result->contract_id,
      'next_payment_date' =>$result->next_payment_date,
      'status' =>$result->status,
      'product_sku' => $result->product_sku,
      'msisdn' => $result->msisdn
    );
    $product_sku = $rows[$result->contract_id]['product_sku'];
    $contract_id = $rows[$result->contract_id]['contract_id'];
    $fields = field_read_fields(array('type' => 'tpay_product'));
    foreach ($fields as $field => $field_name) {
      $field_instance = field_read_instances(array('field_name' => $field));
      $entity_type = $field_instance[0]['entity_type'];
      $query = new EntityFieldQuery();
      $query->entityCondition('entity_type', $entity_type)
      ->fieldCondition($field, 'sku', $product_sku, '=');
      $result = $query->execute();

      if (isset($result[$entity_type])) {
        $entity_ids = array_keys($result[$entity_type]);
        $entity_values = entity_load($entity_type, $entity_ids);
        foreach ($entity_values as $entity_value) {
          $rows[$contract_id] += array(
            'product' => l(isset($entity_value->title) ? $entity_value->title : $entity_value->name,str_replace('_','/',$entity_type).'/'.$entity_ids[0]),
          );
        }
      }
    }
  }

  if(!empty($rows)){
    $message = variable_get('tpay_howtouse_message','');
    if(isset($message['value']) && $message['format'] ) {
      $form['message'] = array(
        '#markup' => check_markup($message['value'],$message['format'])
      );
    }
  }


  $form['user_subscription']['action'] = array(
    '#type' => 'select',
    '#title' => t('Actions'),
    '#options' => array(
      'cancel' => t('Unsubscribe'),
    ),

  );

  $form['user_subscription_table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No Contracts Found'),
    '#attributes' => array('class' => array('table table-bordered table-hover')),
  );

  $form['pager'] = array(
    '#markup' => theme('pager')
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#attributes' => array('class' => array('btn btn-danger')),

  );

  return $form;

}

/**
* Submit tpay_user_subscription_form
* Preform actions to subscriptions e.g. cancellation
*/
function tpay_user_subscription_form_submit($form, &$form_state) {
  $action = $form_state['values']['action'];

  foreach($form_state['values']['user_subscription_table'] as $key => $value){
    if(is_string($value)){
      $values[] = $form_state['complete form']['user_subscription_table']['#options'][$value];
    }
  }

  switch ($action) {
    case 'cancel':
    $tpay = _get_tpay_object();
    foreach($values as $key => $value){
      $contract_id = $value['contract_id'];
      $cancel_contract = $tpay->cancel_subscription_contract_request($contract_id);
      if ($cancel_contract['operationStatusCode'] == 0) {
        $cancel_contracts = db_update('tpay_contracts')
        ->fields(array(
          'status' => 'Cancelled',
        ))
        ->condition('contract_id', $contract_id)
        ->execute();
        drupal_set_message(t('You have successfully unsubscribed'));
      }
      else {
        drupal_set_message(t('Error has occurred'),'error');
        watchdog('TPay','Cancellation Error: !error',array('!error' => $cancel_contract['errorMessage']),WATCHDOG_ERROR);

      }

    }
    break;
  }
}

/**
* Returns a renderable user subscription form
* @see tpay_menu
*/
function tpay_user_subscribe_form ($form, &$form_state, $vars = NULL){
  $form += _tpay_form_element('mobile');
  $form += _tpay_form_element('products_array_radios',$vars);

  $tpay_operator_code = (isset($form_state['values']['tpay_operator_code'])) ? $form_state['values']['tpay_operator_code'] : '60201' ;
  $tpay_product_sku = (isset($form['tpay_product_sku']['#default_value'])) ?  $form['tpay_product_sku']['#default_value'] : $vars;

  $tpay = _get_tpay_object();
  $products = $tpay->get_products();
  $catalogs = $tpay->get_product_catalogs();
  $salesChannels = $tpay->get_sales_channels();

  foreach ($salesChannels as $salesChannel) {
    if ($salesChannel['mobileOperatorCode'] == $tpay_operator_code){
      $sales_channel_id = $salesChannel['salesChannelId'];
      $currency_code = $salesChannel['currencyCode'];
      foreach ($catalogs as $catalog) {
        if ($catalog['salesChannelId'] == $sales_channel_id) {
          $catalog_id = $catalog['catalogId'];

        }
      }
    }
  }

  foreach ($products as $product) {
    if ($product['SKU'] == $tpay_product_sku) {
      $product_name = $product['productName'];
      foreach ($product['pricings'] as $pricing) {
        if ($pricing['catalogId'] == $catalog_id) {
          $price = $pricing['price'];
        }
      }
    }
  }

  $tpay_subscription_freq = variable_get('tpay_subscription_freq','0');
  switch ($tpay_subscription_freq) {
    case 0:
    $subscription_period = t('daily');
    break;
    case 1:
    $subscription_period = t('weekly');
    break;
    case 2:
    $subscription_period = t('monthly');
    break;
    case 3:
    $subscription_period = t('yearly');
    break;
  }

  $_SESSION['subscription_period'] = $subscription_period;

  $form['price'] = array(
    '#markup' => '<div id="price_div"><h4>'.t('You will subscribe in !product_name with !price !currency_code !subscription_period.', array("!product_name" => $product_name,"!price" => $price,"!currency_code" => $currency_code,"!subscription_period" => $subscription_period)).'<br />'.variable_get('tpay_website_unsubscribe','').variable_get('tpay_mobile_unsubscribe','').variable_get($tpay_operator_code).'<br /></div>',
  );

  $_SESSION['price'] = $price;
  $_SESSION['price_info'] = array(
    'currency_code' => $currency_code,
    'form' => $form['price'],
  );

  $form += _tpay_form_element('submit',t('Subscribe'));
  $form['#attributes'] = array('class' => 'form-inline');

  $subscription_details = array();
  global $base_url;
  $subscription_details['product_name'] = $product_name;
  $subscription_details['service_name'] =  variable_get('service_name');
  $subscription_details['website_url'] = $base_url . '/user';
  $subscription_details['price'] = $price.$currency_code;
  $subscription_details['subscription_period'] = $_SESSION['subscription_period'];
  $subscription_details['cancellation_keyword'] = variable_get($tpay_operator_code);
  $_SESSION['subscription_details'] = $subscription_details;
  return $form;

}

function tpay_price_callback ($form, &$form_state) {
  $tpay_operator_code = (isset($form_state['values']['tpay_operator_code'])) ?  $form_state['values']['tpay_operator_code'] : 1;
  $tpay_product_sku = (isset($form_state['values']['tpay_product_sku'])) ?  $form_state['values']['tpay_product_sku'] : 1;
  $subscription_period = $_SESSION['subscription_period'];


  $tpay = _get_tpay_object();
  $products = $tpay->get_products();
  $catalogs = $tpay->get_product_catalogs();
  $salesChannels = $tpay->get_sales_channels();

  foreach ($salesChannels as $salesChannel) {
    if ($salesChannel['mobileOperatorCode'] == $tpay_operator_code){
      $sales_channel_id = $salesChannel['salesChannelId'];
      $currency_code = $salesChannel['currencyCode'];
      foreach ($catalogs as $catalog) {
        if ($catalog['salesChannelId'] == $sales_channel_id) {
          $catalog_id = $catalog['catalogId'];

        }
      }
    }
  }

  foreach ($products as $product) {
    if ($product['SKU'] == $tpay_product_sku) {
      $product_name = $product['productName'];
      foreach ($product['pricings'] as $pricing) {
        if ($pricing['catalogId'] == $catalog_id) {
          $price = $pricing['price'];
        }
      }
    }
  }

  $form['price'] = array(
    '#markup' => '<div id="price_div"><h4>'.t('You will subscribe in !product_name with !price !currency_code !subscription_period.', array("!product_name" => $product_name,"!price" => $price,"!currency_code" => $currency_code,"!subscription_period" => $subscription_period)).'<br />'.variable_get('tpay_website_unsubscribe','').variable_get('tpay_mobile_unsubscribe','').variable_get($tpay_operator_code).'<br /></div>',
  );
  return $form['price'] ;
}

/**
* Validate tpay_user_subscribe_form
* Check if user is allowed to subscribe to multiple products
* Prevent user subscription if already 'Active'
*/
function tpay_user_subscribe_form_validate ($form,&$form_state){
  $prevent_multiple_subscription = module_invoke('content_subscription_tpay','multiple_subscription');
  if($prevent_multiple_subscription){
    $msisdn = $form_state['values']['tpay_mobile_number'];
    $product_sku = $form_state['values']['tpay_product_sku'];
    global $base_url;
    if(_has_contract($msisdn,'anyActive')){
      form_set_error('tpay_mobile_number',t('You already have an active contract, Please follow the !link to access your account', array('!link' => l(t('link'),$base_url . '/user'))));
    }
  }
}

/**
* Submit tpay_user_subscribe_form
* Send add_subscription_contract_request
*/
function tpay_user_subscribe_form_submit ($form,&$form_state){
  global $user;
  $uid = $user->uid;
  $msisdn = $form_state['values']['tpay_mobile_number'];
  $operator_code = $form_state['values']['tpay_operator_code'];
  $subscription_plan_id = variable_get('tpay_subscription_plan_id','');
  $initial_payment_product_id = $form_state['values']['tpay_product_sku'];
  $parking_period = !_has_contract($msisdn,"New",$initial_payment_product_id) ? variable_get('tpay_parking_period','0') : 0;
  $execute_initial_payment_now = !isset($parking_period) ? 'true' : 'false';
  $recurring_payment_product_id = $form_state['values']['tpay_product_sku'];
  $tpay = _get_tpay_object();
  $products = $tpay->get_products();
  $product_catalogs = $tpay->get_product_catalogs();
  $product_sales_channels = $tpay->get_sales_channels();
  foreach($products as $product ) {
    foreach($product['pricings'] as $pricing){
      $catalog_id = $pricing['catalogId'];
      foreach ($product_catalogs as $product_catalog){
        $sales_channel_id = $product_catalog['salesChannelId'];
        foreach ($product_sales_channels as $product_sales_channel){
          $sales_mobile_operator = $product_sales_channel['mobileOperatorCode'];
          if ($sales_mobile_operator == $operator_code){
            $product_catalog_name = $product_catalog['catalogName'];
          }
        }
      }
    }
  }
  $execute_recurring_payment_now = !isset($parking_period) ? 'true' : 'false';
  $contract_start_date = gmdate("Y-m-d H:i:s\Z");
  $initial_payment_date = gmdate("Y-m-d H:i:s\Z", strtotime($parking_period." hours"));
  // $tpay_subscription_freq = variable_get('tpay_subscription_freq','0');
  // // switch ($tpay_subscription_freq) {
  // //   case 0:
  // //    $subscription_period = '+1 day';
  // //   break;
  // //   case 1:
  // //    $subscription_period = '+1 week';
  // //   break;
  // //   case 2:
  // //    $subscription_period = '+1 month';
  // //   break;
  // //   case 3:
  // //    $subscription_period = '+1 year';
  // //   break;
  // // }
  $contract_end_date = gmdate("Y-m-d H:i:s\Z", strtotime("+ 1 year"));
  $auto_renew_contract = variable_get('auto_renew_contract')  ? 'true' : 'false';;
  $language = variable_get('tpay_sms_language','0');
  $send_verification_sms = variable_get('send_verification_sms') ? 'true' : 'false';
  $allow_multiple_free_start_periods = 'true';
  $add_subscription_contract = $tpay->add_subscription_contract_request((string)$uid, $msisdn, $operator_code, $subscription_plan_id, $initial_payment_product_id, $initial_payment_date, $execute_initial_payment_now, $recurring_payment_product_id, $product_catalog_name, $execute_recurring_payment_now, $contract_start_date, $contract_end_date, $auto_renew_contract, $language, $send_verification_sms, $allow_multiple_free_start_periods, $header_enrichment_reference_code = '', $sms_id = '');
  if ($add_subscription_contract['operationStatusCode'] == 0){
    $_SESSION['contract_id'] =$contract_id = $add_subscription_contract['subscriptionContractId'];
    $_SESSION['msisdn'] = $msisdn;
    $_SESSION['operator_code'] = $operator_code;
    $_SESSION['subscription_plan_id'] = $subscription_plan_id;
    $_SESSION['product_sku'] = $initial_payment_product_id;
    $_SESSION['product_catalog_name'] = $product_catalog_name;
    $_SESSION['payment_transaction_status_code'] = $add_subscription_contract['paymentTransactionStatusCode'];
    $_SESSION['transaction_id'] = $add_subscription_contract['transactionId'];
    $_SESSION['payment_date'] = $initial_payment_date;
    $_SESSION['next_payment_date'] = $add_subscription_contract['nextPaymentDate'];
    $exists = db_query('SELECT 1 FROM {tpay_contracts} WHERE contract_id = :contract_id', array(':contract_id' => $contract_id))->fetchField();
    if (!$exists){
      db_insert('tpay_contracts')
      ->fields(array(
        'customer_account_number' => $uid,
        'contract_id' => $_SESSION['contract_id'],
        'msisdn' => $_SESSION['msisdn'],
        'operator_code' => $_SESSION['operator_code'],
        'subscription_plan_id' => $_SESSION['subscription_plan_id'],
        'product_sku' => $_SESSION['product_sku'],
        'product_catalog_name' => $_SESSION['product_catalog_name'],
        'payment_transaction_status_code' => $_SESSION['payment_transaction_status_code'],
        'transaction_id' => $_SESSION['transaction_id'],
        'payment_date' => $_SESSION['payment_date'],
        'next_payment_date' => $_SESSION['next_payment_date'] ,
        'status' => 'New',
      ))
      ->execute();
    }

    drupal_set_message(t("Verification Pin had been sent to ").$msisdn);
    if ($uid==0){
      $form_state['redirect'] = 'tpay/verify/signup';
    } else {
      $form_state['redirect'] = 'tpay/verify';
    }

  }
  else {
    drupal_set_message(t('Error has occurred'),'error');
    watchdog('TPay','Add Subscription Error: !error',array('!error' => $add_subscription_contract['errorMessage']),WATCHDOG_ERROR);

  }
}


/**
* page callback for tpay/verify
* Show verfication page
* @see tpay_menu
*/
function tpay_user_verify_page($path = NULL) {
  return drupal_get_form('tpay_user_verify_form',$path);
}

/**
* Returns a renderable user verify form to be used by drupal_get_form().
*/
function tpay_user_verify_form ($form,&$form_state, $path = NULL){
  if (isset($_SESSION['uid'])){
    $uid = $_SESSION['uid'];
  } else {
    $uid = 0;
  }

  $form = array();

  $price = $_SESSION['price'];
  $currency_code = $_SESSION['price_info']['currency_code'];
  $form['price'] = $_SESSION['price_info']['form'];
  $form += _tpay_form_element('validation_pin');
  $form += _tpay_form_element('submit',t('Confirm Subscription'));
  switch ($path) {
    case 'signup':

    $form['email'] = array(
      '#type' => 'textfield',
      '#title' => t('Email Address'),
      '#size' => 25,
      '#required' => TRUE,
      '#attributes' => array('class' => array('form-control')),

    );
    $form['pass'] = array(
      '#type' => 'password_confirm',
      '#size' => 25,
      '#required' => TRUE,
      '#attributes' => array('class' => array('form-control')),
    );
    $form += _tpay_form_element('login_link');
    if (module_exists('simple_fb_connect')){
      $form += _tpay_form_element('facebook_login');
    }


    break;
    case 'login':

    $form['email'] = array(
      '#type' => 'textfield',
      '#title' => t('Email Address'),
      '#size' => 25,
      '#required' => TRUE,
      '#attributes' => array('class' => array('form-control')),

    );
    $form['pass'] = array(
      '#type' => 'password',
      '#size' => 25,
      '#required' => TRUE,
      '#attributes' => array('class' => array('form-control')),

    );
    $form['pass']['#title'] = t('Password');
    $form += _tpay_form_element('signup_link');
    if (module_exists('simple_fb_connect')){
      $form += _tpay_form_element('facebook_login');
    }

    break;
  }
  $form['tpay_send_verification_sms'] = array(
      '#name' => 'tpay_send_verification_sms',
      '#type' => 'submit',
      '#value' => t('Resend Verification Pin'),
      '#submit' => array('_tpay_send_verification_sms_submit'),
      '#limit_validation_errors' => array(),
      '#attributes' => array('class' => array('btn btn-warning')),
    );
  $form_state['path'] = $path;
  $form['#attributes'] = array('class' => 'form-inline');

  return $form;
}

function tpay_user_verify_form_validate($form, &$form_state){
  global $user;
  $uid = $user->uid;

  if ($uid == 0){
    $path = $form_state['path'];
    $email = $form_state['values']['email'];
    $pass = $form_state['values']['pass'];
    if(valid_email_address($email)){
      switch ($path) {
        case 'login':
        $uid = user_authenticate($email,$pass);
        if ($uid == 0 || empty($uid)) {
          form_set_error('email',t('Invalid information, Please try again'));
        }
        break;
        case 'signup':
        $user_obj = user_load_by_mail($email);
        if (!empty($user_obj)){
          form_set_error('email',t('Email is already registered'));
        }
        break;
      }
    } else {
      form_set_error('email',t('Invalid Email Address, Please provide valid email address'));
    }

  }
}

/**
* Submit tpay_user_verify_form
* Verify user subscription, user_authenticate and register new users
*/
function tpay_user_verify_form_submit ($form,&$form_state, $path = NULL){
  $pin_code = $form_state['values']['tpay_validation_pin'];
  $contract_id = $_SESSION['contract_id'];
  $tpay = _get_tpay_object();
  $verify_contract = $tpay->verify_subscription_contract($contract_id,$pin_code);
  if ($verify_contract['operationStatusCode'] == 0 ){
    if (!user_is_logged_in()){
      $path = $form_state['path'];
      $email = $form_state['values']['email'];
      $pass = $form_state['values']['pass'];
      switch ($path) {
        case 'login':
        $uid = user_authenticate($email,$pass);
        break;
        case 'signup':
        $new_user = array (
          'name' => $email,
          'pass' => $pass,
          'mail' => $email,
          'status' => variable_get('user_register', 1) == 1 ? 1 : 0,
          'roles' => array(
            DRUPAL_AUTHENTICATED_RID => 'authenticated user',
          ),
        );
        user_save('',$new_user);
        $uid = user_authenticate($email,$pass);
        break;
      }
    } else {
      global $user;
      $uid = $user->uid;
    }

    $contract_id = $_SESSION['contract_id'];
    $msisdn = $_SESSION['msisdn'];
    $operator_code = $_SESSION['operator_code'];
    $subscription_plan_id = $_SESSION['subscription_plan_id'];
    $product_sku = $_SESSION['product_sku'];
    $product_catalog_name = $_SESSION['product_catalog_name'];
    $payment_transaction_status_code = $_SESSION['payment_transaction_status_code'];
    $transaction_id = $_SESSION['transaction_id'];
    $initial_payment_date = $_SESSION['payment_date'];
    $next_payment_date = $_SESSION['next_payment_date'];


    $exists = db_query('SELECT 1 FROM {tpay_contracts} WHERE contract_id = :contract_id', array(':contract_id' => $contract_id))->fetchField();
    if (!$exists){
      db_insert('tpay_contracts')
      ->fields(array(
        'customer_account_number' => $uid,
        'contract_id' => $contract_id,
        'msisdn' => $msisdn,
        'operator_code' => $operator_code,
        'subscription_plan_id' => $subscription_plan_id,
        'product_sku' => $product_sku,
        'product_catalog_name' => $product_catalog_name,
        'payment_transaction_status_code' => $payment_transaction_status_code,
        'transaction_id' => $transaction_id,
        'payment_date' => $initial_payment_date,
        'next_payment_date' => $next_payment_date,
        'status' => 'Active',
      ))
      ->execute();
      $data = array();
      $data['action'] = 'UpdateData';
      $data['uid'] = $uid;
      $data['nid'] = $_SESSION['nid'];
      $data['contract_id'] = $contract_id;
      $data['payment_type'] = $_SESSION['payment_type'];
      $data['expiry_date'] = $next_payment_date;
      $data['product_reference'] = $_SESSION['product_reference'];

      module_invoke_all('tpay_contract_renewal',$data);
    }

    else {
      db_update('tpay_contracts')
      ->fields(array(
        'customer_account_number' => $uid,
        'msisdn' => $msisdn,
        'operator_code' => $operator_code,
        'subscription_plan_id' => $subscription_plan_id,
        'product_sku' => $product_sku,
        'product_catalog_name' => $product_catalog_name,
        'payment_transaction_status_code' => $payment_transaction_status_code,
        'transaction_id' => $transaction_id,
        'payment_date' => $initial_payment_date,
        'next_payment_date' => $next_payment_date,
        'status' => 'Active',
      ))
      ->condition('contract_id', $contract_id)
      ->execute();
      $data = array();
      $data['action'] = 'UpdateData';
      $data['uid'] = $uid;
      $data['nid'] = $_SESSION['nid'];
      $data['contract_id'] = $contract_id;
      $data['payment_type'] = $_SESSION['payment_type'];
      $data['expiry_date'] = $next_payment_date;
      $data['product_reference'] = $_SESSION['product_reference'];
      module_invoke_all('tpay_contract_renewal',$data);
    }

    $user_login = array();
    $user_login['uid'] = $uid;
    user_login_submit(array(), $user_login);
    $form_state['redirect'] = 'user/'.$uid.'/tpay';
    drupal_set_message(t('You have successfully subscribed'));
    $subscription_details = $_SESSION['subscription_details'];
    $subscription_details['username'] = $email;
    $subscription_details['password'] = $pass;
    $send_custom_sms = variable_get('send_custom_sms');
    $message_body = t('Thank you for subscribing to !bundle_name of !service_name. You can access your account and enjoy the service by visiting our website !website using your username !username and password !password. You will be charged !price/!subscription_period. To Unsubscribe please log to your account and select Unsubscribe or !cancellation_keyword',
    array(
      '!bundle_name' => $subscription_details['product_name'],
      '!service_name' => $subscription_details['service_name'],
      '!website' => $subscription_details['website_url'],
      '!username' => $subscription_details['username'],
      '!password' => $subscription_details['password'],
      '!price' => $subscription_details['price'],
      '!subscription_period' => $subscription_details['subscription_period'],
      '!cancellation_keyword' => $subscription_details['cancellation_keyword'],
    ));

    if(!empty($message_body) && $send_custom_sms){
      $tpay = _get_tpay_object();
       $send_welcome_sms = $tpay->send_free_mtmessage($message_body,'2'.$msisdn,$operator_code);
      if ($send_welcome_sms['messageDeliveryStatus']){watchdog('TPay','Send SMS Error: !error',array('!error' => $send_welcome_sms['errorMessage']),WATCHDOG_ERROR);}
    }
  } else {
    drupal_set_message(t('Invalid Pin Code'),'error');
    watchdog('TPay','Verfication Error: !error',array('!error' => $verify_contract['errorMessage']),WATCHDOG_ERROR);
  }

}

/*
* Resend Verification Pin SMS
*/
function _tpay_send_verification_sms_submit(){
  $contract_id = $_SESSION['contract_id'];
  $msisdn = $_SESSION['msisdn'];
  $tpay = _get_tpay_object();
  $send_verification_sms = $tpay->send_subscription_contract_verification_sms($contract_id);
  if ($send_verification_sms['operationStatusCode'] == 0 ){drupal_set_message(t("Verification Pin had been sent to ").$msisdn);}
  else {
    drupal_set_message(t('Error has occurred'),'error');
    watchdog('TPay','Send Verfication Error: !error',array('!error' => $send_verification_sms['errorMessage']),WATCHDOG_ERROR);
  }
}

function _has_contract ($msisdn,$status,$product_sku = NULL){
  switch ($status) {
    case 'New':
    $has_contract = db_query('SELECT 1 FROM {tpay_contracts} WHERE msisdn = :msisdn AND product_sku = :product_sku AND status != :status', array(':msisdn' => $msisdn,':product_sku' => $product_sku,':status' => 'New'))->fetchField();
      break;
    case 'anyActive':
    $has_contract = db_query('SELECT 1 FROM {tpay_contracts} WHERE msisdn = :msisdn AND status = :status', array(':msisdn' => $msisdn,':status' => 'Active'))->fetchField();
      break;
  }
  return $has_contract;
}

/*
* Tpay notification
*/
function tpay_notifications(){
  watchdog('TPay',"TPAY Notification",array(),WATCHDOG_INFO);
  $action = $_GET['action'];
  $msg= array();
  $digest = $_GET['digest'];
  switch ($action) {
    case 'SubscriptionChargingNotification':
    case 'SubscriptionCancellationChargingNotification':
      array_push($msg,$action);
      array_push($msg,$_GET['subscriptionContractId']);
      array_push($msg,$_GET['customerAccountNumber']);
      array_push($msg,$_GET['paymentTransactionStatusCode']);
      array_push($msg,$_GET['transactionId']);
      array_push($msg,$_GET['amountCharged']);
      array_push($msg,$_GET['currencyCode']);
      array_push($msg,$_GET['paymentDate']);
      array_push($msg,$_GET['nextPaymentDate']);
      array_push($msg,$_GET['productCatalogName']);
      array_push($msg,$_GET['productId']);
      break;
    case 'SubscriptionContractStatusChanged':
      array_push($msg,$action);
      array_push($msg,$_GET['subscriptionContractId']);
      array_push($msg,$_GET['customerAccountNumber']);
      array_push($msg,$_GET['status']);
      array_push($msg,$_GET['reason']);
      break;
  }

  $tpay = _get_tpay_object();
  $signature =  $tpay->contract_status_change($msg);
  // $msg= array();
  // $digest = null;
  // foreach ($_GET as $key => $value) {
  //   if ($key != 'q'){
  //     if ($key == 'digest'){
  //       $digest = $digest.$value;
  //     } else {
  //       array_push($msg,$value);
  //     }
  //   }
  // }
  //

  if ($digest == $signature) {
    $action = $_GET['action'];
    switch ($action) {
      case 'SubscriptionContractStatusChanged':
      $id = $_GET;
      db_update('tpay_contracts')
      ->fields(array(
        'status' => $_GET['status']
      ))
      ->condition('contract_id', $_GET['subscriptionContractId'])
      ->execute();
      drupal_json_output('Notification Recieved');

      break;
      case 'SubscriptionChargingNotification':

        db_update('tpay_contracts')
        ->fields(array(
          'payment_transaction_status_code' => $_GET['paymentTransactionStatusCode'],
          'transaction_id' => $_GET['transactionId'],
          'amount_charged' => $_GET['amountCharged'],
          'currency_code' => $_GET['currencyCode'],
          'payment_date' => $_GET['paymentDate'],
          'next_payment_date' => $_GET['nextPaymentDate'],

        ))
        ->condition('contract_id', $_GET['subscriptionContractId'])
        ->execute();
        $data = array();
        $data['action'] = 'RenewExpiryDate';
        $data['contract_id'] = $_GET['subscriptionContractId'];
        $data['expiry_date'] = $_GET['nextPaymentDate'];
        module_invoke_all('tpay_contract_renewal',$data);
        break;
        drupal_json_output('Notification Recieved');
      }
    }
    else {
      watchdog('TPay',"Notification Error: !error",array('!error'=>'Access Denied'),WATCHDOG_CRITICAL);
      drupal_access_denied();
    }

  }
