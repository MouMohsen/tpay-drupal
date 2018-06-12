<?php

/**
* @file
* Provide views data and handlers for node.module.
*
* @ingroup views_module_handlers
*/

/**
* Implements hook_views_data().
*/
function tpay_views_data() {
  $data['tpay_contracts']['table']['group'] = t('TPay Subscriptions');
  $data['tpay_contracts']['table']['base'] = array(
    'field' => 'customer_account_number',
    'title' => t('TPay Subscriptions'),
  );
  $data['tpay_contracts']['table']['join'] = array(
    // Index this array by the table name to which this table refers.
    'field_data_field_tpay_product' => array(
      // The primary key in the referenced table.
      'left_field' => 'field_tpay_product_sku',
      // The foreign key in this table.
      'field' => 'product_sku',
    ),
  );
  $data['tpay_contracts']['customer_account_number'] = array(
    'title' => t('UID'),
    'help' => t('TPay User ID '),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'relationship' => array(
      'title' => t('User'),
      'help' => t('User Data'),
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'customer_account_number',
      'label' => t('user'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'customer_account_number',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['tpay_contracts']['contract_id'] = array(
    'title' => t('Contract ID'),
    'help' => t('TPay Contract ID'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'contract_id',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['tpay_contracts']['msisdn'] = array(
    'title' => t('Mobie Number'),
    'help' => t('User Mobile Number '),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'msisdn',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['tpay_contracts']['subscription_plan_id'] = array(
    'title' => t('Subscription Plan ID'),
    'help' => t('TPay Subscription Plan ID'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'subscription_plan_id',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $tpay_field_name = key(field_read_fields(array('type' => 'tpay_product')));
  $tpay_field_table_name = 'field_data_'.$tpay_field_name;
  $tpay_field_column_name = $tpay_field_name.'_sku';

  foreach (field_read_instances(array('field_name' => $tpay_field_name)) as $field ) {
    $entity_type = $field['entity_type'];
    $entity_info = entity_get_info($entity_type);
  }

  $data['tpay_contracts']['product_sku'] = array(
    'title' => t('Product SKU'),
    'help' => t('TPay Product ID/SKU'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'relationship' => array(
      'title' => t('TPay Products'),
      'help' => t('entities associated with TPay as Products'),
      'handler' => 'views_handler_relationship_entity_reverse',
      'field_name' => $tpay_field_name,
      'field table' => $tpay_field_table_name,
      'field field' => $tpay_field_column_name,
      'base' => $entity_info['base table'],
      'base field' => $entity_info['entity keys']['id'],
      'label' => t('TPay Products'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'validate type' => 'product_sku',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['tpay_contracts']['payment_date'] = array(
    'title' => t('Payment Date'),
    'help' => t('Initial tpay payment date'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_date',
      'numeric' => TRUE,
      'validate type' => 'payment_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['tpay_contracts']['status'] = array(
    'title' => t('Status'),
    'help' => t('Status of subscriptions'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'numeric' => False,
      'validate type' => 'status',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );



  return $data;

}
