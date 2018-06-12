<?php

class Tpay {

  const TPAY_SUBSCRIPTION_ENDPOINT = 1;
  const TPAY_MANAGEMENT_ENDPOINT = 2;
  const TPAY_MESSAGE_ENDPOINT = 3;


  public $username;
  public $publickey;
  public $privatekey;
  public $staging_mode = false;

  function __construct($username, $publickey, $privatekey) {
    $this->username = $username;
    $this->publickey = $publickey;
    $this->privatekey = $privatekey;
  }

  function get_mobile_operators() {
    $ops = $this->call_tpay(
      'GetMobileOperators',
      array(
        'username' => $this->username,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops['GetMobileOperatorsResult'];
  }

  function get_sales_channels() {
    $ops = $this->call_tpay(
      'GetSalesChannels',
      array(
        'username' => $this->username,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops['GetSalesChannelsResult'];
  }

  function get_product_catalogs() {
    $ops = $this->call_tpay(
      'GetProductCatalogs',
      array(
        'username' => $this->username,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops['GetProductCatalogsResult'];
  }

  function add_product_catalog($sales_channel_id,$name,$is_enabled) {
    $ops = $this->call_tpay(
      'AddProductCatalog',
      array(
        'salesChannelId' => $sales_channel_id,
        'name' => $name,
        'isEnabled' => $is_enabled,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops['AddProductCatalogResult'];
  }

  function update_product_catalog($product_catalog_id,$name,$is_enabled) {
    $ops = $this->call_tpay(
      'UpdateProductCatalog',
      array(
        'productCatalogId' => $product_catalog_id,
        'name' => $name,
        'isEnabled' => $is_enabled,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops;
  }

  function delete_product_catalog($product_catalog_id) {
    $ops = $this->call_tpay(
      'DeleteProductCatalog',
      array(
        'catalogId' => $product_catalog_id,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops;
  }

  function get_products() {
    $ops = $this->call_tpay(
      'GetProducts',
      array(
        'username' => $this->username,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops['GetProductsResult'];
  }

  function add_product($name,$sku,$is_enabled,$pricings) {
    $pricings_string= json_encode($pricings);
    $ops = $this->call_tpay(
      'AddProduct',
      array(
        'name' => $name,
        'sku' => $sku,
        'isEnabled' => $is_enabled
      ),
      self::TPAY_MANAGEMENT_ENDPOINT,
      array(
        'pricings' => $pricings,
      )
    );
    return $ops['AddProductResult'];
  }

  function update_product($product_id,$name,$sku,$is_enabled,$pricings) {
    $ops = $this->call_tpay(
      'UpdateProduct',
      array(
        'productId' => $product_id,
        'name' => $name,
        'sku' => $sku,
        'isEnabled' => $is_enabled
      ),
      self::TPAY_MANAGEMENT_ENDPOINT,
      array(
        'pricings' => $pricings,
      )
    );
    return $ops;
  }

  function delete_product($product_id) {
    $ops = $this->call_tpay(
      'DeleteProduct',
      array(
        'productId' => $product_id,
      ),
      self::TPAY_MANAGEMENT_ENDPOINT
    );
    return $ops;
  }

  function add_subscription_contract_request($customer_account_number, $msisdn, $operator_code, $subscription_plan_id, $initial_payment_product_id, $initial_payment_date, $execute_initial_payment_now, $recurring_payment_product_id, $product_catalog_name, $execute_recurring_payment_now, $contract_start_date, $contract_end_date, $auto_renew_contract, $language, $send_verification_sms, $allow_multiple_free_start_periods, $header_enrichment_reference_code = '', $sms_id = '') {
    $ops = $this->call_tpay(
      'AddSubscriptionContractRequest',
      array(
        "customerAccountNumber" => $customer_account_number,
        "msisdn" => $msisdn,
        "operatorCode" => $operator_code,
        "subscriptionPlanId" => $subscription_plan_id,
        "initialPaymentproductId" => $initial_payment_product_id,
        "initialPaymentDate" => $initial_payment_date,
        "executeInitialPaymentNow" => $execute_initial_payment_now,
        "recurringPaymentproductId" => $recurring_payment_product_id,
        "productCatalogName" => $product_catalog_name,
        "executeRecurringPaymentNow" => $execute_recurring_payment_now,
        "contractStartDate" => $contract_start_date,
        "contractEndDate" => $contract_end_date,
        "autoRenewContract" => $auto_renew_contract,
        "language" => $language,
        "sendVerificationSMS" => $send_verification_sms,
        "allowMultipleFreeStartPeriods" => $allow_multiple_free_start_periods,
        "headerEnrichmentReferenceCode" => $header_enrichment_reference_code,
        "smsId" => $sms_id,
      ),
      self::TPAY_SUBSCRIPTION_ENDPOINT
    );
    return $ops;
  }

  function verify_subscription_contract($subscription_contract_id, $pin_code) {
    $ops = $this->call_tpay(
      'VerifySubscriptionContract',
      array(
        "subscriptionContractId"=>$subscription_contract_id,
        "pinCode" => $pin_code,
      ),
      self::TPAY_SUBSCRIPTION_ENDPOINT
    );
    return $ops;
  }

  function send_subscription_contract_verification_sms($subscription_contract_id) {
    $ops = $this->call_tpay(
      'SendSubscriptionContractVerificationSMS',
      array(
        "subscriptionContractId" => $subscription_contract_id,
      ),
      self::TPAY_SUBSCRIPTION_ENDPOINT
    );
    return $ops;
  }

  function send_free_mtmessage($message_body,$msisdn,$operator_code) {
    $ops = $this->call_tpay(
      'SendFreeMTMessage',
      array(
        "messageBody"=>$message_body,
        "msisdn"=>$msisdn,
        "operatorCode"=>$operator_code,
      ),
      self::TPAY_MESSAGE_ENDPOINT
    );
    return $ops;
  }

//Cancel Subscription Contract Request Directly
  function cancel_subscription_contract_request($subscription_contract_id) {
    $ops = $this->call_tpay(
      'CancelSubscriptionContractRequest',
      array(
        "subscriptionContractId"=>$subscription_contract_id,
      ),
      self::TPAY_SUBSCRIPTION_ENDPOINT
    );
    return $ops;
  }

  function send_subscription_cancellation_pin_sms($subscription_contract_id) {
    $ops = $this->call_tpay(
      'SendSubscriptionCancellationPinSMS',
      array(
        "subscriptionContractId"=>$subscription_contract_id,
      ),
      self::TPAY_SUBSCRIPTION_ENDPOINT
    );
    return $ops;
  }

  function verify_subscription_cancellation_pin($subscription_contract_id,$pin_code) {
    $ops = $this->call_tpay(
      'VerifySubscriptionCancellationPin',
      array(
        "subscriptionContractId"=>$subscription_contract_id,
        "pinCode" => $pin_code,
      ),
      self::TPAY_SUBSCRIPTION_ENDPOINT
    );
    return $ops;
  }

  function contract_status_change($msg){
    $signature =  $this->get_signature($msg);
    return $signature;
  }

  private function get_signature($message_array) {
    $message = implode($message_array);
    return $this->publickey.":".hash_hmac("sha256",$message,$this->privatekey);
  }

  private function call_tpay($command, $data, $endpoint, $arguments = NULL) {
    $endpoint_path = $endpoint == self::TPAY_SUBSCRIPTION_ENDPOINT ? 'TPAYSubscription': ($endpoint == self::TPAY_MANAGEMENT_ENDPOINT ? 'TPAYCatalogs' : 'TPAY' );
    if (is_null($data)) { $data = array(); }
    $signature =  $this->get_signature($data);
    if (!is_null($arguments)) {
      $data = array_merge($data, $arguments);
    }
    $data['signature'] = $signature;
    $data_string = json_encode($data);
    $tpay_server = $this->staging_mode ? 'staging.tpay.me' : 'live.tpay.me';
    $ch = curl_init('http://' . $tpay_server . '/api/' . $endpoint_path . '.svc/json/' . $command);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
    curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array(
      'Content-Type: application/json',
      'Content-Length: ' . strlen($data_string),
    ));
    $result = curl_exec($ch);
    $obj = json_decode($result, TRUE);
    return $obj;
  }
}
