const loginElement = document.querySelector('#login-form');
const topnav = document.querySelector('#topnav');
const main = document.querySelector('#main');
const cmdClean = document.querySelector('#cmd-clean');
const climateVars = document.querySelector('#climate-vars');
const sideBar = document.querySelector('#side-bar');
//const containerElem = document.querySelector('#container'); 
//const settings = document.querySelector('#settings');
const contentElement = document.querySelector('#content-sign-in');
const userDetailsElement = document.querySelector('#user-details'); 
const authBarElement = document.querySelector('#authentication-bar');


// Elements for sensor readings
const tempElement = document.getElementById("temp");
const humElement = document.getElementById("hum");
const presElement = document.getElementById("pres");

// MANAGE LOGIN/LOGOUT UI
const setupUI = (user) => {
  if (user) {     //user is logged in 
    //toggle UI elements
    loginElement.style.display = 'none';    //hide login form 
    topnav.style.display = 'block';         //show topnav
    main.style.display = 'block';           //show main
    contentElement.style.display = 'block';   //show user info
    authBarElement.style.display ='block';    
    cmdClean.style.display = 'block';
    climateVars.style.display = 'block';
    sideBar.style.display = 'block';
    userDetailsElement.innerHTML = user.email;
    userDetailsElement.style.display ='block';
    

    /*window.onLoad = function displayUserEmail() {
      userDetailsElement.innerHTML = user.email;
    }*/
    

    // get user UID to get data from database
    var uid = user.uid;
    console.log(uid);

    // Database paths (with user UID)
    var dbPathTemp = 'UsersData/' + uid.toString() + '/temperature';
    var dbPathHum = 'UsersData/' + uid.toString() + '/humidity';
    var dbPathPres = 'UsersData/' + uid.toString() + '/pressure';

    // Database references
    var dbRefTemp = firebase.database().ref().child(dbPathTemp);
    var dbRefHum = firebase.database().ref().child(dbPathHum);
    var dbRefPres = firebase.database().ref().child(dbPathPres);

    // Update page with new readings
    dbRefTemp.on('value', snap => {
      tempElement.innerText = snap.val().toFixed(2);
    });

    dbRefHum.on('value', snap => {
      humElement.innerText = snap.val().toFixed(2);
    });

    dbRefPres.on('value', snap => {
      presElement.innerText = snap.val().toFixed(2);
    });

  // if user is logged out
  } else{
    // toggle UI elements
    console.log("hello")
    loginElement.style.display = 'block';   //show loginForm 
    authBarElement.style.display ='none';
    userDetailsElement.style.display ='none';
    topnav.style.display = 'block';       //show top-nav 
    main.style.display = 'none';
    cmdClean.style.display = 'none';
    climateVars.style.display = 'none';
    sideBar.style.display = 'none';
    //contentElement.style.display = 'none';
  }
}