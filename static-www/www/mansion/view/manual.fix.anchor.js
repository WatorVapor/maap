import * as Vue from 'https://cdn.jsdelivr.net/npm/vue@3.2.37/dist/vue.esm-browser.prod.js';
const COORD = await import(`/maap/assets/js/gps/Coord.js`);
console.log('::COORD=<',COORD,'>');
const coord = new COORD.Coord();

document.addEventListener('DOMContentLoaded', async (evt) => {
  tryLoadManualFix_();
});

const fixed_anchor_option = {
  data() {
    const manualFixGpsStr = localStorage.getItem(constAnchorGpsManualFix);
    if(manualFixGpsStr) {
      const manualFixGps = JSON.parse(manualFixGpsStr);
      console.log('fixed_anchor_option::manualFixGps=<',manualFixGps,'>');
      return {
        anchors:manualFixGps.anchor
      };
    } else {
      const posHistory = loadMapBySavedGpsAnchors();
      console.log('fixed_anchor_option::posHistory=<',posHistory,'>');
      for(const address in posHistory.anchor) {
        console.log('fixed_anchor_option::address=<',address,'>');
        const anchor = posHistory.anchor[address];
        console.log('fixed_anchor_option::anchor=<',anchor,'>');
        const xyz = coord.WGS2ECEF(anchor.lat,anchor.lon,anchor.geo);
        console.log('fixed_anchor_option::xyz=<',xyz,'>');
        anchor.x = xyz.x;
        anchor.y = xyz.y;
        anchor.z = xyz.z;
      }
      console.log('fixed_anchor_option::posHistory.anchor=<',posHistory.anchor,'>');
      return {
        anchors:posHistory.anchor
      };
    }
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

const gVueApp = {};
const tryLoadManualFix_ = () => {
  if(!gVueApp.fixAnchor) {
    const appAnchor = Vue.createApp(fixed_anchor_option);
    gVueApp.fixAnchor = appAnchor.mount('#ui-vue-fix-anchor-manual');
  }
}
