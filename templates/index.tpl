{literal}
<script type="text/javascript">
	function check(box) {
		boxes = 0;
		files = document.getElementsByName("file[]");
		for (file in files) {
			boxes+=(files[file].checked)?1:0;
		}
		/*if (boxes > 2) {
			box.checked = false;
			boxes--;
		}*/
		but = document.getElementById('cmp_button');
		if (boxes == 2 || boxes == 1) {
			but.disabled = false;
		} else {
			but.disabled = true;
		}
	}
</script>
<script type="text/javascript" src="js/sorttable.js"></script>
{/literal}

<!-- file upload -->
<form enctype="multipart/form-data" action="upload.php" method="POST">
	<input type="hidden" name="MAX_FILE_SIZE" value="20000000" />
	Image file to upload (Max. file size: 15 MB): <input name="uploadedfile" type="file" /><br />
	<b>Supported formats are: TIFF, JPEG, DjVu, JPEG2000.</b><br />
	<input type="submit" value="Upload File" />
</form>
<!-- images -->
<form action="index.php" method="GET">
	<table class="sortable">
		<tr>
			<th></th>
			<th>file</th>
			<th>extension</th>
			<th>uploaded</th>
			<th>size (MB)</th>
		</tr>
		{foreach from=$files item=file}
		<tr>
			<td><input type="checkbox" name="file[]" value="{$file.name}" onchange="javascript:check(this);"></td>
			<td>{$file.name}</td>
			<td>&nbsp;&nbsp;&nbsp;&nbsp;{$file.ext}</td>
			<td>&nbsp;&nbsp;{$file.modified}</td>
			<td>&nbsp;&nbsp;&nbsp;{$file.size}</td>
		</tr>
		{/foreach}
	</table>
	<input id="cmp_button" type="submit" name="action" value="execute" disabled="true"/>
	<input id="delete_images" type="submit" name="action" value="delete_images"/>
	<input id="delete_all_files" type="submit" name="action" value="delete_all_files" />
</form>
<form>
	<hr />
{if $profiles|@count gt 0}
	<table class="sortable">
		<tr>
			<th></th>
			<th>file</th>
			<th>modified</th>
		</tr>
	{foreach from=$profiles item=profile}
		<tr>
			<td><input type="checkbox" name="profile[]" value="{$profile.name}"></td>
			<td><a href="profiles/{$profile.name}">{$profile.name}</a></td>
			<td>{$profile.modified}</td>
		</tr>
	{/foreach}
	</table>
	<input id="delete_profiles" type="submit" name="action" value="delete_selected_profiles" />
	<input id="delete_all_profiles" type="submit" name="action" value="delete_all_profiles" />
{/if}
</form>
<!--
<form action="index.php" method="GET">
	<input type="hidden" name="action" value="delete_all_profiles" />
	<input type="submit" value="Delete all profiles" />
</form>
-->
<!-- error -->
{if $error}
<h1>{$error}</h1>
{/if}
