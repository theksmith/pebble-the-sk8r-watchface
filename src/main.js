Pebble.addEventListener('ready', function(e) {
	//...
});


Pebble.addEventListener("showConfiguration", function(e) {
	//todo: finish config screen
	
	var watchinfo;
	
	if (Pebble.getActiveWatchInfo) {
		watchinfo = Pebble.getActiveWatchInfo(); 
	} else {
		watchinfo = {};
	}
	
	watchinfo = {};
	
	Pebble.openURL('http://theksmith.com/x/pebble-config/' + '?platform=' + watchinfo.platform + '&account=' + Pebble.getAccountToken());
});


Pebble.addEventListener("webviewclosed", function(e) {
	//...
});