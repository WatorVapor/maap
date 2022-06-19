const onUISettingInfo = (info) => {
  APP.vm.wifi = info.wifi;
  APP.vm.mqtt = info.mqtt;
}
document.addEventListener('DOMContentLoaded',()=>{
  setTimeout(()=>{
    createVueApps();
  },1000);
})
const APP = {};
const setting_anchor_data = {
  wifi:{
    password: '',
    ssid: '',
  },
  mqtt:{
    address: '',
    jwt: '',
    url: '',
  }
}
const createVueApps = ()=> {
  const setting_option = {
    data() {
      return setting_anchor_data;
    },
    methods: {
      onClickChangeSetting(evt) {
        console.log('onClickChangeSetting::evt=<',evt,'>');
        console.log('onClickChangeSetting::this.wifi=<',this.wifi,'>');
        console.log('onClickChangeSetting::this.mqtt=<',this.mqtt,'>');
        const setting = {
          setting:{
            wifi:this.wifi,
            mqtt:this.mqtt
          }
        }
        writeJsonCmd(setting);
      },
    }
  };
  const settingApp = Vue.createApp(setting_option);
  APP.vm = settingApp.mount('#vue-ui-info-of-achor');  
}