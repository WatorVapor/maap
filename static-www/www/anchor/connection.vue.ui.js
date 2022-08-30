import * as Vue from 'https://cdn.jsdelivr.net/npm/vue@3.2.37/dist/vue.esm-browser.prod.js';
const onConnectiongInfo = (info) => {
  if(info.wifi) {
    connectionApp.vm.wifi = info.wifi;
  }
  if(info.mqtt) {
    connectionApp.vm.mqtt = info.mqtt;
  }
  if(info.topic) {
    connectionApp.vm.topic = info.topic;
  }
  if(info.uwb) {
    connectionApp.vm.uwb = info.uwb;
  }
}

document.addEventListener('DOMContentLoaded',()=>{
  setTimeout(()=>{
    createVueConnectionApp();
  },1000);
})
const connectionApp = {};
const setting_connection_data = {
  wifi:{
    password: '',
    ssid: '',
  },
  mqtt:{
    address: '',
    jwt: '',
    url: '', 
  },
  topic:{
    out:[],
  },
  uwb:{
    mode:0,
    id:0,
  },
  mansions:[],
  mansionSelected:'',
}
const createVueConnectionApp = ()=> {  
  const setting_option = {
    data() {
      return setting_connection_data;
    },
    methods: {
      onClickChangeSetting(evt) {
        //console.log('onClickChangeSetting::evt=<',evt,'>');
        //console.log('onClickChangeSetting::this.wifi=<',this.wifi,'>');
        //console.log('onClickChangeSetting::this.mqtt=<',this.mqtt,'>');
        const uri = URI.parse(this.mqtt.url);
        const jwt = URI.parse(this.mqtt.jwt);
        //console.log('onClickChangeSetting::uri=<',uri,'>');
        //console.log('onClickChangeSetting::jwt=<',jwt,'>');
        const setting = {
          setting:{
            wifi:this.wifi,
            mqtt:this.mqtt,
            uwb:this.uwb,
            mqtt_:{
              url:{
                host:uri.host,
                port:uri.port,
                path:uri.resourceName,
              },
              jwt:{
                host:jwt.host,
                port:jwt.port,
                path:jwt.resourceName,
              }
            }
          }
        }
        writeJsonCmd(setting);
      },
      onClickAddMansion(evt) {
        console.log('onClickAddMansion::evt=<',evt,'>');
        console.log('onClickAddMansion::this.mansionSelected=<',this.mansionSelected,'>');
        if(!this.topic) {
          this.topic = {};
        }
        if(!this.topic.out) {
          this.topic.out = [];
        }
        this.topic.out.push(this.mansionSelected);
        this.topic.out = [...new Set(this.topic.out)];
        console.log('onClickAddMansion::this.topic.out=<',this.topic.out,'>');
        const setting = {
          setting:{
            topic:{
              out:this.topic.out
            }
          }
        }
        writeJsonCmd(setting);
      },
    }
  };
  const settingApp = Vue.createApp(setting_option);
  connectionApp.vm = settingApp.mount('#vue-ui-connection-of-anchor');  
  const starMansionListStr = localStorage.getItem(constMansionList);
  if(starMansionListStr) {
    const starMansionList = JSON.parse(starMansionListStr);    
    console.log('loadStarryDashboardApp_::starMansionList=<',starMansionList,'>');
    connectionApp.vm.mansions = starMansionList;
  }
}
