<?php

require("config.php");
require("common.php");
$config = config();

$smarty = smarty_init();

$smarty->assign("profiles", $profiles);
$smarty->assign('template', 'faq.tpl');
$smarty->assign('selected', 'faq');
$smarty->display('layout.tpl');
