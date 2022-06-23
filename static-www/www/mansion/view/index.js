const appPrefix = '/maap';
const loadFrameWorkJS = ()=> {
  const fileref = document.createElement('script');
  fileref.setAttribute('type','text/javascript');
  fileref.setAttribute('src', `${appPrefix}/assets/js/app.js`);
  document.getElementsByTagName('head')[0].appendChild(fileref);
}
document.addEventListener('DOMContentLoaded', (evt) => {
  loadFrameWorkJS();
});
