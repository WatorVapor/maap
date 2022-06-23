const GRAVITON = await import(`./assets/js/gravity/graviton.js`);
document.addEventListener('AppScriptLoaded', async (evt) => {
  console.log('AppScriptLoaded::evt=<',evt,'>');
  loadStarRgionsApp_(evt);
});
const loadStarRgionsApp_ = (evt) => {
  console.log('AppScriptLoaded::loadStarRgionsApp_=<',loadStarRgionsApp_,'>');  
}
