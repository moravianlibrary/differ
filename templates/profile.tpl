<form>
  <table>
     <tr>
       <td></td>
       <td>Name</td>
       <td><input type="input" name="name" value="test"></td>
     </tr>
     {foreach from=$parameters item=value key=key}
     <tr>
       <td><input type="checkbox" name="{$key}" value="on"></td>
       <td>{$key}</td>
       <td><input type="input" name="{$key}_value" value="{$value.value}"></td>
     </tr>
     {/foreach}
  </table>
  <input type="hidden" name="op" value="create"/>
  <input type="submit" value="Create profile"/>
</form>
