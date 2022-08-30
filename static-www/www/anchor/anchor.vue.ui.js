import * as Vue from 'https://cdn.jsdelivr.net/npm/vue@3.2.37/dist/vue.esm-browser.prod.js';

const onUIAnchor = (anchor) => {
}

document.addEventListener('DOMContentLoaded',()=>{
  setTimeout(()=>{
    createVueAnchorApp();
  },1000);
})
const anchorApp = {};
const setting_anchor_data = {
  uwb:{
    id:1
  }
}
const createVueAnchorApp = ()=> {  
  const setting_option = {
    data() {
      return setting_anchor_data;
    },
    methods: {
      onClickChangeAnchorMode(evt) {
        console.log('onClickChangeAnchorMode::evt=<',evt,'>');
        const anchorCmd = {
          uwb:{
            AT:`AT+anchor_tag=0,${this.uwb.id} \r\n`
          }
        }
        writeJsonCmd(anchorCmd);
      },
      onClickChangeTagId(evt) {
        console.log('onClickChangeTagId::evt=<',evt,'>');
        const anchorCmd = {
          uwb:{
            AT:`AT+anchor_tag=1,${this.uwb.id} \r\n`
          }
        }
        writeJsonCmd(anchorCmd);
      },
    }
  };
  const settingApp = Vue.createApp(setting_option);
  anchorApp.vm = settingApp.mount('#vue-ui-uwb-of-anchor');  
}
