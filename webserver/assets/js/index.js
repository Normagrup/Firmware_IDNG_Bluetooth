function openLogin() 
{
	var login = document.getElementById('login');
	var loginOverlay = document.getElementById('loginOverlay');

	login.style.visibility = "visible";
	loginOverlay.style.visibility = "visible";
}

function closeLoginPopup() 
{
	var login = document.getElementById('login');
	var loginOverlay = document.getElementById('loginOverlay');
	var signErrorLabel = document.getElementById('signError');

	login.style.visibility = "hidden";
	loginOverlay.style.visibility = "hidden";
	signErrorLabel.style.visibility = "hidden";
}

function loadPage(page)
{
	askStateToEmbedded(page);
}

function loadPageAfterAsk(page, answer)
{
	var frame = window.top.document.getElementById("mainframe");

	if(answer === "FREE") { frame.src = page; }
	else if(answer === "SCAN") {
		frame.src = "s_nodes_old.html";
		setTimeout(function() {
			confirmScan("", true);
		}, 300);
	}
	else if(answer === "COMMISSION") {
		frame.src = "s_nodes_old.html";
		setTimeout(function() {
			processIsCommissionInProgress("");
		}, 300);
	}
	else if(answer === "ADD_MANUAL") {
		frame.src = "s_nodes_old.html";
		setTimeout(function() {
			processIsAddingManualInProgress("");
		}, 300);
	}
	else if(answer === "REPLACE") {
		frame.src = "s_nodes_old.html";
		setTimeout(function() {
			processIsReplacingInProgress("");
		}, 300);
	}
	else if(answer === "DEL_DEV_BC") {
		frame.src = "s_nodes.html";
		setTimeout(function() {
			processDelAllDev("", true);
			showToast(); // variable
		}, 300);
	}
	else if(answer === "DEL_DEV") {
		frame.src = "s_nodes.html";
		setTimeout(function() {
			processDelOneDev("", true);
		}, 300);
	}
	else if(answer === "SETTER_RELAY") {
		frame.src = "s_nodes.html";
		setTimeout(function() {
			processSetRelayInProgress("");
			processEstimatedTime("0:0:5");
		}, 300);
	}
	else if(answer === "ADD_NODE_TO_GROUP") {
		frame.src = "s_groups.html";
		setTimeout(function() {
			addToGroupVisual();
		}, 300);
	}
	else if(answer === "DEL_NODE_FROM_GROUP") {
		frame.src = "s_groups.html";
		setTimeout(function() {
			delFromGroupVisual();
		}, 300);
	}
	else if(answer === "DEL_COMPLETE_GROUP") {
		frame.src = "s_groups.html";
		setTimeout(function() {
			delGroupVisual();
			showToast(); // variable
		}, 300);
	}
	else if(answer === "CLEAR_ALL") {
		frame.src = "s_general_config.html";
		setTimeout(function() {
			clearAllDataVisual();
			showToast(); // variable
		}, 300);
	}
	else if(answer === "SYNC_POL") {
		frame.src = "s_power_on_level.html";
		setTimeout(function() {
			syncPOLVisual();
			showToast(); // variable
		}, 300);
	}
	else if(answer === "SCAN_BY_NODE") {
		frame.src = "s_nodes_old.html";
		setTimeout(function() {
			confirmScan("", true);
		}, 300);
	}
	else if(answer === "LINE_SCAN") {
		frame.src = "s_general_config.html";
		setTimeout(function() {
			processIsLSInProgress("");
		}, 300);
	}
	else if(answer === "APPLY_AUTOASSIGNMENT") {
		frame.src = "s_assignment.html";
		setTimeout(function() {
			processApplyAssign("", true);
		}, 300);
	}
	else if(answer === "GROUP_AUTOASSIGNMENT") {
		frame.src = "s_groups.html";
		setTimeout(function() {
			processGroupAutoAssign("", true);
		}, 300);
	}
	else if(answer === "SCANNING_SERIAL") {
		frame.src = "s_assignment.html";
		setTimeout(function() {
			processScanSerial("", true);
		}, 300);
	}

	requestDateTime();
}
