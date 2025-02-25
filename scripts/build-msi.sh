#!/usr/bin/env bash
PRODUCTNAME="OpenRGB Hardware Sync Plugin"
PN_SANS_WS=${PRODUCTNAME//\ /_}
VENDOR="OpenRGB"

TLD="org"
WEBSITE="https://${VENDOR,,}.${TLD}"
NAMESPACE=$(uuidgen -n @url -N ${WEBSITE} --sha1 | awk '{ print toupper($0) }')
VENDOR_ID="${TLD}.${VENDOR}"
APP_ID="${VENDOR_ID}.${PRODUCTNAME,,}"

GITURL="https://gitlab.com/OpenRGBDevelopers/OpenRGBHardwareSyncPlugin/-"
GITPARAM="?inline=false"
ICONFILE="OpenRGBHardwareSyncPlugin.ico"
LICENSEFILE="scripts/License.rtf"
BANNERIMAGE="scripts/banner.bmp"
DIALOGBACKGROUND="scripts/dialog_background.bmp"
PROJECT_FILE="OpenRGBHardwareSyncPlugin.pro"
XMLSUFFIX=".wxs"
XMLOUTFILE=${PN_SANS_WS}${XMLSUFFIX}


VERSION=$(qmake ${PROJECT_FILE} 2>&1 | grep VERSION_WIX | cut -d ':' -f 3 | tr -d ' ')
#If the suffix is not empty set the channel to "-git"
CHANNEL=$(grep SUFFIX\  ${PROJECT_FILE} | cut -d= -f 2)
if [[ -n $CHANNEL ]];
then
    CHANNEL="-git"
fi


#The Upgrade code has to be consistent to allow upgrades between channels
#This value is roughly equivalent to "provides" in Linux packaging
UPGRADECODE=$(uuidgen -n ${NAMESPACE} -N ${APP_ID} --sha1 | awk '{ print toupper($0) }')
#The ProductID will be unique per build
PRODUCTCOMMENT="Open source RGB lighting control that doesn't depend on manufacturer software."

#Print Metadata to the log
echo -e "Icon URL:\t" $GITURL$ICONFILE
echo -e "License URL:\t" $GITURL$LICENSEFILE
echo -e "AppID - Channel:\t" ${APP_ID} " - " ${CHANNEL}
echo -e "Upgrade UUID:\t" ${UPGRADECODE}
echo -e "Product Name:\t" ${PRODUCTNAME}
echo -e "Vendor - VendorID:\t\t" ${VENDOR} " - " ${VENDOR_ID}
echo -e "Version:\t" ${VERSION}


#Wix and / or Wine have issues with the mixed upper and lower case letters
WORKING_PATH="orgb/"
BUILT_PATH="OpenRGBHardwareSyncPlugin.dll"
mkdir ${WORKING_PATH}
mv "${BUILT_PATH}" ${WORKING_PATH}

EXTENSION="orp"
SAVE_FILE="${PRODUCTNAME}.${EXTENSION}"
count=1

for file in "$WORKING_PATH"/*;
do
    filename=$(basename "$file")
    if [ $filename == "${PN_SANS_WS}.exe" ] ; then
        #If this is the executable treat as a special case as we need the reference for later
        EXE_ID=${PRODUCTNAME}00
        EXE_FILE=${filename}
        #Add special entry to files list
        FILES="$FILES                            <File Id='${EXE_ID}' Source='${WORKING_PATH}${filename}'/>\n"
    elif [ -d "$file" ] ; then
            #If this is a directory then we need to add another component
            COMPONENTS="${COMPONENTS}                <ComponentRef Id='${filename}Files'/>\n"
            TEMP="                        <Directory Id='${filename}' Name='${filename}'>\n                            <Component Id='${filename}Files' Guid='"$(uuidgen -t | awk '{ print toupper($0) }')"'>\n"
            for file2 in "$file"/*;
            do
                filename2=$(basename "$file2")
                TEMP="$TEMP                                <File Id='${PRODUCTNAME}${count}' Source='${WORKING_PATH}${filename}/${filename2}'/>\n"
                count=$((count+1))
            done
            DIRECTORIES="$DIRECTORIES$TEMP                            </Component>\n                        </Directory>\n"
    else
            #Any other file to files list
            FILES="$FILES                            <File Id='${PN_SANS_WS}${count}' Source='${WORKING_PATH}${filename}'/>\n"
            count=$((count+1))
    fi
done

#############################################################
# Create the Wix XML file                                   #
#   Set IFS to the empty string to allow arbitrary spacing  #
#   within the XML_FILE variable, then set it back to a     #
#   space character after we print the XML file             #
#############################################################
IFS=""
XML_FILE=""

XML_FILE+="<?xml version='1.0' encoding='windows-1252'?>\r\n"
XML_FILE+="<Wix xmlns='http://schemas.microsoft.com/wix/2006/wi'>\r\n"
XML_FILE+="    <Product Name='${PRODUCTNAME}' Manufacturer='${VENDOR}'\r\n"
XML_FILE+="        Id='*'\r\n"
XML_FILE+="        UpgradeCode='"${UPGRADECODE}"'\r\n"
XML_FILE+="        Language='1033' Codepage='1252' Version='${VERSION}'>\r\n"
XML_FILE+="        <Package Keywords='Installer' Description='${PRODUCTNAME} Installer'\r\n"
XML_FILE+="            Comments=\"${PRODUCTCOMMENT}\" Manufacturer='OpenRGB'\r\n"
XML_FILE+="            InstallerVersion='200' Languages='1033' Compressed='yes' SummaryCodepage='1252' Platform='x64'/>\r\n"
XML_FILE+="        <Media Id='1' Cabinet='${PRODUCTNAME,,}.cab' EmbedCab='yes'/>\r\n"
XML_FILE+="        <Condition Message='This package supports Windows 64bit Only'>VersionNT64</Condition>\r\n"
#XML_FILE+="        <MajorUpgrade Schedule='afterInstallInitialize' AllowDowngrades='yes'/>\r\n"
XML_FILE+="        <Icon Id='OpenRGBIcon' SourceFile='${ICONFILE}'/>\r\n"
XML_FILE+="        <Property Id='ARPPRODUCTICON' Value='OpenRGBIcon'/>\r\n"
XML_FILE+="        <Property Id='ARPURLINFOABOUT' Value='https://www.openrgb.org'/>\r\n"
XML_FILE+="        <Property Id='WIXUI_INSTALLDIR' Value='INSTALLDIR'/>\r\n"
XML_FILE+="        <UIRef Id='WixUI_InstallDir'/>\r\n"
XML_FILE+="        <UIRef Id='WixUI_ErrorProgressText'/>\r\n"
XML_FILE+="        <WixVariable Id='WixUILicenseRtf' Value='${LICENSEFILE}'/>\r\n"
XML_FILE+="        <WixVariable Id='WixUIBannerBmp' Value='${BANNERIMAGE}'/>\r\n"
XML_FILE+="        <WixVariable Id='WixUIDialogBmp' Value='${DIALOGBACKGROUND}'/>\r\n"
XML_FILE+="\r\n"
XML_FILE+="        <Directory Id='TARGETDIR' Name='SourceDir'>\r\n"
XML_FILE+="            <Directory Id='ProgramFiles64Folder'>\r\n"
XML_FILE+="                <Directory Id='${VENDOR}' Name='${VENDOR}'>\r\n"
XML_FILE+="                    <Directory Id='INSTALLDIR' Name='plugins'>\r\n"
XML_FILE+="                        <Component Id='${PN_SANS_WS}Files' Guid='"$(uuidgen -t | awk '{ print toupper($0) }')"'>\r\n"
XML_FILE+="${FILES}\r\n"
XML_FILE+="                        </Component>\r\n"
XML_FILE+="${DIRECTORIES}\r\n"
XML_FILE+="                    </Directory>\r\n"
XML_FILE+="                </Directory>\r\n"
XML_FILE+="            </Directory>\r\n"
XML_FILE+="        </Directory>\r\n"
XML_FILE+="        <Feature Id='Complete' Title='${PRODUCTNAME}' Description='Install all ${PRODUCTNAME} files.' Display='expand' Level='1' ConfigurableDirectory='INSTALLDIR'>\r\n"
XML_FILE+="            <Feature Id='${PN_SANS_WS}Complete' Title='${PRODUCTNAME}' Description='The complete package.' Level='1' AllowAdvertise='no' InstallDefault='local'>\r\n"
XML_FILE+="                <ComponentRef Id='${PN_SANS_WS}Files'/>\r\n"
XML_FILE+="${COMPONENTS}\r\n"
XML_FILE+="            </Feature>\r\n"
XML_FILE+="        </Feature>\r\n"
XML_FILE+="    </Product>\r\n"
XML_FILE+="</Wix>"

echo -e $XML_FILE > $XMLOUTFILE
IFS=" "

#############################################################
# Print the XML for debugging                               #
#############################################################
cat $XMLOUTFILE

#############################################################
# Once the XML file manifest is created, create the package #
#############################################################
candle -arch x64 ${XMLOUTFILE}
light -sval -ext WixUIExtension ${PN_SANS_WS}.wixobj -out ${PN_SANS_WS}_Windows_64.msi
