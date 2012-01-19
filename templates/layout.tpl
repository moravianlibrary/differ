<!DOCTYPE html>
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
		<title>NDK Image Data Validator</title>
		<link type="text/css" href="main.css" rel="stylesheet" />	
	</head>
	<body><!-- Logo a nadpis -->
		<h1  style="color:white; font-size:45px; display:inline; float:right">NDK Image Data Validator</h1>	
		<span style="text-align:right; position:relative; top:0px; margin:0px 30px 0px 0px;">
			<a href="http://www.nkp.cz">
				<img src="image/logo.jpg" width="117px" height="82px" border="none">
			</a>
		</span>
		<!-- Tabs - Hlavni okno --> 
		<div id="#tabs" class="ui-tabs ui-widget ui-widget-content ui-corner-all">
		<!-- MENU -->
			<ul id="tabs" class="ui-tabs ui-tabs-nav ui-helper-reset ui-helper-clearfix ui-widget-header ui-corner-all">
				{assign var="sel_class" value="ui-state-default ui-corner-top ui-tabs-selected ui-state-active ui-state-focus ui-state-hover"}
				{assign var="unsel_class" value="ui-state-default ui-corner-top ui-state-hover"}
				<li class='{if $selected=="about"}{$sel_class}{else}{$unsel_class}{/if}'>
					<a href="about.php">About</a>
				</li>
				<li class='{if $selected=="files"}{$sel_class}{else}{$unsel_class}{/if}'><a href="index.php">File Manager</a></li>
				<li class='{if $selected=="compare"}{$sel_class}{else}{$unsel_class}{/if}'><a href="javascript:return false;">Compare</a></li>
				<li class='{if $selected=="profile"}{$sel_class}{else}{$unsel_class}{/if}'><a href="profile.php">Profile creator</a></li>
				<li class='{if $selected=="faq"}{$sel_class}{else}{$unsel_class}{/if}'><a href="faq.php">FAQ</a></li>
				<li class='{if $selected=="documentation"}{$sel_class}{else}{$unsel_class}{/if}'><a href="doc.php">Documentation</a></li>
				<li class='{if $selected=="help"}{$sel_class}{else}{$unsel_class}{/if}' style="float:right; margin-right: -530px;">
					<a href="help.php">Help</a>
				</li>
				<li class='{if $selected=="traffic"}{$sel_class}{else}{$unsel_class}{/if}'><a href="traffic.php">Traffic</a></li>
				<li class='{if $selected=="tos"}{$sel_class}{else}{$unsel_class}{/if}'><a href="tos.php">TOS</a></li>
			</ul>
			<div id="#tabs-1" class="divpage ui-tabs-panel ui-widget-content ui-corner-bottom">
				{include file="$template"}
			</div>
		<br class="clear" />
		<br/>
		</div>
		<div align="center">©2011-2012 National Library of the Czech republic | <a href="tos.php">Terms of service</a> </div>
	<script type="text/javascript">
	{literal}
	var _gaq = _gaq || [];
		_gaq.push(['_setAccount', 'UA-27726498-1']);
		_gaq.push(['_trackPageview']);
	(function() {
		var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
		ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
		var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
	})();
	{/literal}
	</script>
	</body>
</html>
