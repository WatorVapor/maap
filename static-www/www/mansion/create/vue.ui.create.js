document.addEventListener('AppScriptLoaded', async (evt) => {
  createStarMansionApp_();
});
const GSM = await import(`${appPrefix}/assets/js/gravity/mansion.js`);

const star_mansion_option = {
  data() {
    return {
      mansion:{
        address:'',
        name:''
      }
    };
  },
  methods: {
    onStarMansionSaveBtn(evt) {
      //console.log('createStarMansionApp_::evt=<',evt,'>');
      //console.log('createStarMansionApp_::this.mansion=<',this.mansion,'>');
      onStarMansionSave(this.mansion,this.instance);
    }
  }  
}
import * as Vue from 'https://cdn.jsdelivr.net/npm/vue@3.2.37/dist/vue.esm-browser.prod.js';

const createStarMansionApp_ = async ()=> {
  console.log('createStarMansionApp_::GSM=<',GSM,'>');
  const mansion = new GSM.StarMansion(constCreateMansionPrefix);
  console.log('createStarMansionApp_::mansion=<',mansion,'>');
  console.log('createStarMansionApp_::mansion=<',mansion,'>');
  const address = mansion.address();
  console.log('createStarMansionApp_::address=<',address,'>');
  const appStarMansion = Vue.createApp(star_mansion_option);
  const mansionVM = appStarMansion.mount('#vue-ui-create-star-mansion');
  mansionVM.mansion.address = address;
  mansionVM.mansion.name = `star-mansion_${address}`;
  mansionVM.instance = mansion;
}

const onStarMansionSave = (mansionUI,mansion) => {
  console.log('createStarMansionApp_::mansionUI=<',mansionUI,'>');
  console.log('createStarMansionApp_::mansion=<',mansion,'>');
  const mf = new GSM.MansionFactory();
  const mansionObj = {
    name:mansionUI.name,
    address:mansion.mass_.address_,
    core:{
      secretKey:mansion.mass_.priKeyB64_,
      publicKey:mansion.mass_.pubKeyB64_,
    }
  };
  mf.save(mansionObj);
  location.reload();  
}