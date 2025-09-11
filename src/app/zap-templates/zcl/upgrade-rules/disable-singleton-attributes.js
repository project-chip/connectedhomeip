/**
 * Upgrade rule to notify users that no attributes in matter have singleton enabled.
 * @param {*} api
 * @param {*} context
 * @returns A string mentioning the singleton changes to cluster attributes made on a .zap file
 */
async function postLoad(api, context)
{
  let resMsg = ''
  let epts   = await api.endpoints(context)
  for (let i = 0; i < epts.length; i++)
  {
    let clusters = await api.clusters(context, epts[i])
    for (let j = 0; j < clusters.length; j++)
    {
        let attributes = await api.attributes(context, epts[i], clusters[j])
        for (let k = 0; k < attributes.length; k++) {
            // None of the matter attributes are singleton
            if (parseInt(attributes[k].isSingleton) == 1 || attributes[k].isSingleton == true) {
                let params = [
                {
                    key: context.updateKey.attributeSingleton,
                    value: false
                }]
                await api.updateAttribute(
                context,
                epts[i],
                clusters[j],
                attributes[k],
                params
                )
                resMsg += `${attributes[k].name} attribute of ${clusters[j].name} cluster on endpoint ${epts[i].endpointIdentifier} is no longer a singleton attribute.\n`
            }
        }
    }
  }
  // Status can be 'nothing', 'automatic', 'user_verification', 'impossible'.
  if (resMsg) {
    console.log("ZAP file upgraded with the following changes:\n" + resMsg);
    return { message : resMsg, status : 'automatic' };
  }
}

exports.postLoad = postLoad