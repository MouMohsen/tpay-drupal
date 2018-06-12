<?php
/**
 * Page callback: TPay settings
 *
 * @see tpay_menu()
 */
function tpay_admin_form($form, &$form_state) {
  $form['tpay_service_messages'] = array(
    '#type' => 'fieldset',
    '#title' => t('TPay Service Messages'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['tpay_service_messages']['tpay_howtouse_message'] = array(
    '#type' => 'text_format',
    '#title' => t('How To Use Service/Welcome Message'),
    '#wysiwyg' => true,
    '#default_value' => variable_get('tpay_howtouse_message')['value'],
    '#format' => variable_get('tpay_howtouse_message')['format'],

  );
  $form['tpay_service_messages']['tpay_welcome_sms_disclaimer'] = array(
    '#type' => 'fieldset',
    '#title' => t('TPay Welcome SMS/Disclaimer'),
    '#collapsible' => FALSE,
    '#collapsed' => TRUE,
  );
  $form['tpay_service_messages']['tpay_welcome_sms_disclaimer']['send_custom_sms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send Custom SMS'),
    '#description' => t('Send Custom SMS that include username, password, website URL, Custom cancellation keywords'),
    '#default_value' => variable_get('send_custom_sms',TRUE),
  );
  $form['tpay_service_messages']['tpay_welcome_sms_disclaimer']['service_name'] = array(
    '#type' => 'textfield',
    '#title' => t('TPay Service Name'),
    '#description' => t('TPay Service Name'),
    '#default_value' => variable_get('service_name'),
  );
  $form['tpay_service_messages']['tpay_welcome_sms_disclaimer']['tpay_website_unsubscribe'] = array(
    '#type' => 'textarea',
    '#title' => t('Website Unsubscribe Disclaimer'),
    '#description' => t('e.g. To cancel using website, please go to '),
    '#default_value' => variable_get('tpay_website_unsubscribe'),
  );
  $form['tpay_service_messages']['tpay_welcome_sms_disclaimer']['tpay_mobile_unsubscribe'] = array(
    '#type' => 'textarea',
    '#title' => t('Mobile Unsubscribe Disclaimer'),
    '#description' => t('e.g. To cancel using mobile'),
    '#default_value' => variable_get('tpay_mobile_unsubscribe'),
  );
  $tpay = _get_tpay_object();
  $operators = $tpay->get_mobile_operators();
  if($operators){
    $operators_options = array();
    foreach ($operators as $operator) {
      $form['tpay_service_messages']['tpay_welcome_sms_disclaimer'][$operator['operatorCode']] = array(
        '#type' => 'textfield',
        '#title' => $operator['countryName'] . " - " . $operator['operatorName'] . t(' Cancellation Message/Keyword'),
        '#description' => t('Send [Keyword] to [short code]'),
        '#default_value' => variable_get($operator['operatorCode']),
      );
      $operators_options[$operator['operatorCode']] = $operator['countryName'] . " - " . $operator['operatorName'];
    }
  }

  $form['tpay_config'] = array(
    '#type' => 'fieldset',
    '#title' => t('TPay Server Configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['tpay_config']['tpay_staging_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Operate in staging mode.'),
    '#default_value' => variable_get('tpay_staging_mode'),
  );
  $form['tpay_config']['tpay_username'] = array(
    '#type' => 'textfield',
    '#title' => t('User name'),
    '#default_value' => variable_get('tpay_username'),
    '#size' => 128,
    '#maxlength' => 128,
    '#description' => t('Your TPay account user name'),
  );
  $form['tpay_config']['tpay_publickey'] = array(
    '#type' => 'textfield',
    '#title' => t('Public key'),
    '#default_value' => variable_get('tpay_publickey'),
    '#size' => 128,
    '#maxlength' => 128,
    '#description' => t('Your TPay account public key'),
  );
  $form['tpay_config']['tpay_privatekey'] = array(
    '#type' => 'textfield',
    '#title' => t('Private key'),
    '#default_value' => variable_get('tpay_privatekey'),
    '#size' => 128,
    '#maxlength' => 128,
    '#description' => t('Your TPay account private key'),
  );
  $form['tpay_subscription_plan'] = array(
    '#type' => 'fieldset',
    '#title' => t('TPay Subscription Plan Configuration'),
    '#description' => t('Using TPay Portal, Create subscription plan and copy its Info. here'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['tpay_subscription_plan']['tpay_subscription_plan_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Subscription Plan ID'),
    '#description' => t('TPay subscription plan ID'),
    '#size' => 20,
    '#maxlength' => 20,
    '#default_value' => variable_get('tpay_subscription_plan_id'),
  );
  $form['tpay_subscription_plan']['tpay_subscription_freq'] = array(
    '#type' => 'select',
    '#description'=> t('Subscription Plan Frequency'),
    '#title' => t('Subscription Plan Frequency'),
    '#options' => array(
      0 => 'Daily',
      1 => 'Weekly',
      2 => 'Monthly',
      3 => 'Yearly'
    ),
  );
  $form['tpay_subscription_plan']['tpay_parking_period'] = array(
    '#type' => 'textfield',
    '#title' => t('Parking Period'),
    '#description' => t('Free start Period for new users(without charging) in hours'),
    '#size' => 20,
    '#maxlength' => 20,
    '#default_value' => variable_get('tpay_parking_period'),
  );
  $form['tpay_subscription_plan']['tpay_sms_language'] = array(
    '#type' => 'select',
    '#description'=> t('Select SMS Language'),
    '#title' => t('SMS Language'),
    '#options' => array(
      1 => 'English',
      2 => 'Arabic',
      3 => 'French',
    ),
    '#default_value' => variable_get('tpay_sms_language'),

  );
  $form['tpay_subscription_plan']['auto_renew_contract'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto renew Contract'),
    '#default_value' => variable_get('auto_renew_contract',1),
  );
  $form['tpay_subscription_plan']['send_verification_sms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send verfication SMS'),
    '#default_value' => variable_get('send_verification_sms',1),
  );

  return system_settings_form($form);
}
