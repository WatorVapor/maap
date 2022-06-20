document.addEventListener('AppScriptLoaded', async (evt) => {
  console.log('AppScriptLoaded::evt=<',evt,'>');
  loadCosmosApp_(evt);
});
const loadCosmosApp_ = (evt) => {
  console.log('loadCosmosApp_::evt=<',evt,'>');  
}
