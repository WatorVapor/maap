const componentTag = 'w-navbar';
const componentURL = `${constAppPrefix}/assets/component/navbar.html`;
const componentData = {
  root:constAppPrefix,
  accout:{ 
    name:''
  }
};
const { default:createMaapVueApp } = await import(`${constAppPrefix}/assets/component/component.js`);
const createVueApp = async ()=> {
  return await createMaapVueApp(componentTag,componentURL,componentData);
}
export default createVueApp;

