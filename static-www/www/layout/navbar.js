const NAVBAR = {
  debug:false,
}
window.addEventListener('DOMContentLoaded', async (evt) => {
  createTopNavBar_();
});

const createTopNavBar_ = async ()=> {
  const { default:createVueApp } = await import(`${constAppPrefix}/assets/component/navbar.js`);
  if(NAVBAR.debug) {
    console.log('w-navbar::createTopNavBar_::createVueApp=<',createVueApp,'>');
  }
  const app = await createVueApp();
  const vm = app.mount('#vue-navbar-top');  
  if(NAVBAR.debug) {
    console.log('w-navbar::createTopNavBar_::vm=<',vm,'>');
  }
  
  const { EDAuth } = await import(`${constAppPrefix}/assets/js/edauth.js`);
  if(NAVBAR.debug) {
    console.log('w-navbar::createTopNavBar_::EDAuth=<',EDAuth,'>');
  }
  const edAuth = new EDAuth();
  vm.accout.name = edAuth.name();
  const evt = document.createEvent('Event');
  evt.initEvent('TopMenuBarLoaded', true, true);
  document.dispatchEvent(evt);
}
