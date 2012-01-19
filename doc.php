<?php

require("config.php");
require("common.php");
$config = config();

$smarty = smarty_init();

$smarty->assign("profiles", $profiles);
$smarty->assign('template', 'doc.tpl');
$smarty->assign('selected', 'documentation');
$smarty->display('layout.tpl');
