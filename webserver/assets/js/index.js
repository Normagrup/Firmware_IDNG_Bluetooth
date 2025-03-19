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
	var frame = window.top.document.getElementById("mainframe");
	frame.src = page;
}
