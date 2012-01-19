<?php

require("config.php");
require("common.php");
$config = config();

$smarty = smarty_init();

$smarty->assign("profiles", $profiles);
$smarty->assign('template', 'traffic.tpl');
$smarty->assign('selected', 'traffic');
$smarty->display('layout.tpl');
