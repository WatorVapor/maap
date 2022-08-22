import * as Vue from 'https://cdn.jsdelivr.net/npm/vue@3.2.37/dist/vue.esm-browser.prod.js';
const COORD = await import(`/maap/assets/js/gps/Coord.js`);
console.log('::COORD=<',COORD,'>');
const coord = new COORD.Coord();

document.addEventListener('AppScriptLoaded', async (evt) => {
  tryLoadManualFix_();
});

const fixed_anchor_option = {
  data() {
    return {
      anchors:{}
    };
  },
  methods: {
    onClickGotoBtn (evt) {
      console.log('onClickGotoBtn::evt=<',evt,'>');
    },
    onChangeAnchorWGS(evt,address) {
      console.log('onChangeAnchorWGS::evt=<',evt,'>');
      console.log('onChangeAnchorWGS::address=<',address,'>');
    },
    onChangeAnchorECEF(evt,address) {
      console.log('onChangeAnchorECEF::evt=<',evt,'>');
      console.log('onChangeAnchorECEF::address=<',address,'>');
    },
  }  
}

const tryLoadManualFix_ = () => {
  const appAnchor = Vue.createApp(fixed_anchor_option);
  const vmAnchor = appAnchor.mount('#ui-vue-fix-anchor-manual');
  const manualFixGpsStr = localStorage.getItem(constAnchorGpsManualFix);
  if(manualFixGpsStr) {
    const manualFixGps = JSON.parse(manualFixGpsStr);
    console.log('tryLoadManualFix_::manualFixGps=<',manualFixGps,'>');  
    vmAnchor.anchors = manualFixGps.anchor;
  } else {
    const posHistory = loadMapBySavedGpsAnchors();
    console.log('tryLoadManualFix_::posHistory=<',posHistory,'>');
    for(const address in posHistory.anchor) {
      console.log('tryLoadManualFix_::address=<',address,'>');
      const anchor = posHistory.anchor[address];
      console.log('tryLoadManualFix_::anchor=<',anchor,'>');
      const xyz = coord.WGS2ECEF(anchor.lat,anchor.lon,anchor.geo);
      console.log('tryLoadManualFix_::xyz=<',xyz,'>');
      anchor.x = xyz.x;
      anchor.y = xyz.y;
      anchor.z = xyz.z;
    }
    vmAnchor.anchors = posHistory.anchor;
  }

}
