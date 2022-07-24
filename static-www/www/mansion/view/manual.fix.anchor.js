const COORD = await import(`/maap/assets/js/gps/Coord.js`);
console.log('::COORD=<',COORD,'>');
const coord = new COORD.Coord();

document.addEventListener('AppScriptLoaded', async (evt) => {
  tryLoadManualFix_();
});

const tryLoadManualFix_ = () => {
  const manualFixGpsStr = localStorage.getItem(constAnchorGpsManualFix);
  if(manualFixGpsStr) {
    const manualFixGps = JSON.parse(manualFixGpsStr);
    console.log('tryLoadManualFix_::manualFixGps=<',manualFixGps,'>');  
  } else {
    const posHistory = loadMapBySavedGpsAnchors();
    console.log('tryLoadManualFix_::posHistory=<',posHistory,'>');
  }
}
