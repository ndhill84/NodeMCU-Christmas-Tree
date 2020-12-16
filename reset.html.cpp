#include <pgmspace.h>

char reset_html[] PROGMEM = R"=====(
<!doctype html>
<html lang='en' dir='ltr'>
<head>
  <title>Christmas Tree Wifi Setup</title>
<body>
<h1>Clear Christmas Tree Network Config?</h1>
<FORM action="/reset" method="post">
<P>
<label>Yes: </label>
<input type="checkbox" name="reset" value="1"><br>
<input type="submit" value="Send"> <INPUT type="reset">
</P>
</FORM>
</body>
</html>
)=====";
