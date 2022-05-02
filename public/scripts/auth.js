var dbRef = firebase.database().ref();

// listen for auth status changes
auth.onAuthStateChanged(user => {
 if (user) {
   console.log("user logged in");
   dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").update({'isUserConnected':true});
   console.log(user);
   setupUI(user);
   var uid = user.uid;
   console.log(uid);
 } else {
   console.log("user logged out");
   dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").update({'isUserConnected':false});
   setupUI();
 }
});

var loggedIn = false;
dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("isUserConnected").on('value', function(snapshot) {
  if (snapshot.val() == false) {
      loggedIn = false;
  }
  else {
      loggedIn = true;
  }
});

// login
const loginForm = document.querySelector('#login-form');
loginForm.addEventListener('submit', (e) => {
 e.preventDefault();
 // get user info
 const email = loginForm['input-email'].value;
 const password = loginForm['input-password'].value;
 // log the user in
if (loggedIn == false) {
  auth.signInWithEmailAndPassword(email, password).then((cred) => {
    // close the login modal & reset form
    loginForm.reset();
    console.log(email);
  })
  .catch((error) =>{
    const errorCode = error.code;
    const errorMessage = error.message;
    document.getElementById("error-message").innerHTML = errorMessage;
    console.log(errorMessage);
  });
}
else {
  document.getElementById("error-message").innerHTML = "A user connected to this system is currently logged in.";
}
});

// logout
const logout = document.querySelector('#logout-link');
logout.addEventListener('click', (e) => {
 e.preventDefault();
 auth.signOut();
});