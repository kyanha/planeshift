/*
* pawslauncherwindow.cpp - Author: Mike Gist
*
* Copyright (C) 2007 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation (version 2 of the License)
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <psconfig.h>
#include <fstream>

#include <iutil/stringarray.h>

#include "paws/pawsbutton.h"
#include "paws/pawscheckbox.h"
#include "paws/pawscombo.h"
#include "paws/pawsimagedrawable.h"
#include "paws/pawslistbox.h"
#include "paws/pawstextbox.h"
#include "paws/pawswidget.h"
#include "paws/pawsyesnobox.h"

#include "globals.h"
#include "pawslauncherwindow.h"

using namespace CS::Threading;
using namespace std;

pawsLauncherWindow::pawsLauncherWindow()
{
}

bool pawsLauncherWindow::PostSetup()
{
    configFile.AttachNew(new csConfigFile(LAUNCHER_CONFIG_FILENAME, psLaunchGUI->GetVFS()));
    configUser.AttachNew(new csConfigFile("/planeshift/userdata/planeshift.cfg", psLaunchGUI->GetVFS()));

    launcherMain = FindWidget("LauncherMain");
    launcherSettings = FindWidget("LauncherSettings");
    launcherUpdater = FindWidget("LauncherUpdater");
    resolution = (pawsComboBox*)FindWidget("ScreenResolution");

    launcherMain->OnGainFocus();

    // Load game settings.
    LoadSettings();

    // Get server news.
    newsUpdater.AttachNew(new Thread(new NewsUpdater(this), true));

    // Setup update available window.
    updateAvailable = (pawsYesNoBox*)FindWidget("UpdateAvailable");
    updateAvailable->SetCallBack(HandleUpdateButton, updateAvailable, "An update to PlaneShift is available. Do you wish to update now?");
    updateAvailable->SetAlwaysOnTop(true);

    return true;
}

void pawsLauncherWindow::UpdateNews()
{
    pawsMessageTextBox* serverNews = (pawsMessageTextBox*)FindWidget("ServerNews");
    psLaunchGUI->GetDownloader()->DownloadFile(configFile->GetStr("Launcher.News.URL", ""), "servernews", true, false);
    
    ifstream newsFile("servernews", ifstream::in);
    csString buffer;
    while(newsFile.good())
    {
        buffer.Append((char)newsFile.get());
    }
    buffer.Truncate(buffer.Length()-1);
    serverNews->AddMessage(buffer.GetDataSafe());
    newsFile.close();
    psLaunchGUI->GetFileUtil()->RemoveFile("servernews");
}

pawsButton* pawsLauncherWindow::FindButton(WidgetID id)
{
    return static_cast<pawsButton*>(FindWidget(id));
}

bool pawsLauncherWindow::OnButtonPressed(int mouseButton, int keyModifier, pawsWidget* widget)
{
    int ID = widget->GetID();

    if(ID == QUIT_BUTTON)
    {
        psLaunchGUI->Quit();
    }
    else if(ID == PLAY_BUTTON)
    {
        psLaunchGUI->ExecClient(true);
        psLaunchGUI->Quit();
    }
    else if(ID == SETTINGS_BUTTON)
    {
        launcherMain->Hide();
        launcherSettings->Show();
        launcherSettings->OnGainFocus();
        FindButton(SETTINGS_GENERAL_BUTTON)->SetState(true, false);
    }
    else if(ID == REPAIR_BUTTON)
    {
        psLaunchGUI->PerformRepair();
        pawsMessageTextBox* output = (pawsMessageTextBox*)FindWidget("UpdaterOutput");
        output->Clear();
        launcherMain->Hide();
        launcherUpdater->Show();
        launcherUpdater->OnGainFocus();
    }
    else if(ID == UPDATER_YES_BUTTON)
    {
        widget->Hide();
        FindWidget("UpdaterNoButton")->Hide();
        FindWidget("UpdaterCancelButton")->Show();
        psLaunchGUI->PerformUpdate(false, true);
    }
    else if(ID == UPDATER_NO_BUTTON)
    {
        FindWidget("UpdaterYesButton")->Hide();
        widget->Hide();
        launcherUpdater->Hide();
        launcherMain->Show();
        launcherMain->OnGainFocus();
        psLaunchGUI->PerformUpdate(false, false);
    }
    else if(ID == UPDATER_OK_BUTTON)
    {
        FindWidget("UpdaterNoButton")->Hide();
        FindWidget("UpdaterYesButton")->Hide();
        widget->Hide();
        launcherUpdater->Hide();
        launcherMain->Show();
        launcherMain->OnGainFocus();
    }
    else if(ID == UPDATER_CANCEL_BUTTON)
    {
        widget->Hide();
        launcherUpdater->Hide();
        launcherMain->Show();
        launcherMain->OnGainFocus();
        psLaunchGUI->CancelUpdater();
    }
    else if(ID == SETTINGS_CANCEL_BUTTON)
    {
        FindWidget("SettingsAudio")->Hide();
        FindWidget("SettingsControls")->Hide();
        FindWidget("SettingsGeneral")->Show();
        FindWidget("SettingsGraphics")->Hide();
        FindButton(SETTINGS_AUDIO_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_CONTROLS_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GRAPHICS_BUTTON)->SetState(false, false);
        launcherSettings->Hide();
        launcherMain->Show();
        launcherMain->OnGainFocus();
    }
    else if(ID == SETTINGS_OK_BUTTON)
    {
        FindWidget("SettingsAudio")->Hide();
        FindWidget("SettingsControls")->Hide();
        FindWidget("SettingsGeneral")->Show();
        FindWidget("SettingsGraphics")->Hide();
        FindButton(SETTINGS_AUDIO_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_CONTROLS_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GRAPHICS_BUTTON)->SetState(false, false);
        launcherSettings->Hide();
        launcherMain->Show();
        launcherMain->OnGainFocus();
        SaveSettings();
        LoadSettings();
    }
    else if(ID == SETTINGS_AUDIO_BUTTON)
    {
        FindWidget("SettingsAudio")->Show();
        FindWidget("SettingsControls")->Hide();
        FindWidget("SettingsGeneral")->Hide();
        FindWidget("SettingsGraphics")->Hide();
        FindButton(SETTINGS_AUDIO_BUTTON)->SetState(true, false);
        FindButton(SETTINGS_CONTROLS_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GENERAL_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GRAPHICS_BUTTON)->SetState(false, false);
        launcherSettings->OnGainFocus();
    }
    else if(ID == SETTINGS_CONTROLS_BUTTON)
    {
        FindWidget("SettingsAudio")->Hide();
        FindWidget("SettingsControls")->Show();
        FindWidget("SettingsGeneral")->Hide();
        FindWidget("SettingsGraphics")->Hide();
        FindButton(SETTINGS_AUDIO_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_CONTROLS_BUTTON)->SetState(true, false);
        FindButton(SETTINGS_GENERAL_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GRAPHICS_BUTTON)->SetState(false, false);
        launcherSettings->OnGainFocus();
    }
    else if(ID == SETTINGS_GENERAL_BUTTON)
    {
        FindWidget("SettingsAudio")->Hide();
        FindWidget("SettingsControls")->Hide();
        FindWidget("SettingsGeneral")->Show();
        FindWidget("SettingsGraphics")->Hide();
        FindButton(SETTINGS_AUDIO_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_CONTROLS_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GENERAL_BUTTON)->SetState(true, false);
        FindButton(SETTINGS_GRAPHICS_BUTTON)->SetState(false, false);
        launcherSettings->OnGainFocus();
    }
    else if(ID == SETTINGS_GRAPHICS_BUTTON)
    {
        FindWidget("SettingsAudio")->Hide();
        FindWidget("SettingsControls")->Hide();
        FindWidget("SettingsGeneral")->Hide();
        FindWidget("SettingsGraphics")->Show();
        FindButton(SETTINGS_AUDIO_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_CONTROLS_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GENERAL_BUTTON)->SetState(false, false);
        FindButton(SETTINGS_GRAPHICS_BUTTON)->SetState(true, false);
        launcherSettings->OnGainFocus();
    }

    return true;
}

void pawsLauncherWindow::HandleUpdateButton(bool choice, void *updatewindow)
{
    pawsWidget* updateWindow = (pawsWidget*)updatewindow;
    psLaunchGUI->PerformUpdate(choice, false);
    updateWindow->Hide();
}

void pawsLauncherWindow::HandleAspectRatio(csString ratio)
{
    const char* current = resolution->GetSelectedRowString();
    resolution->Clear();

    if(ratio == "17:9")
    {
        resolution->NewOption("2048x1080");
        if(!resolution->Select(current))
        {
            resolution->Select("2048x1080");
        }
    }
    else if(ratio == "16:10")
    {
        resolution->NewOption("2560x1600");
        resolution->NewOption("1920x1200");
        resolution->NewOption("1680x1050");
        resolution->NewOption("1440x900");
        resolution->NewOption("1280x800");
        resolution->NewOption("320x200");
        if(!resolution->Select(current))
        {
            resolution->Select("1920x1200");
        }
    }
    else if(ratio == "16:9")
    {
        resolution->NewOption("1920x1080");
        resolution->NewOption("1366x768");
        resolution->NewOption("1280x720");
        resolution->NewOption("854x480");
        if(!resolution->Select(current))
        {
            resolution->Select("1920x1080");
        }
    }
    else if(ratio == "5:4")
    {
        resolution->NewOption("2560x2048");
        resolution->NewOption("1280x1024");
        if(!resolution->Select(current))
        {
            resolution->Select("1280x1024");
        }
    }
    else if(ratio == "5:3")
    {
        resolution->NewOption("1280x768");
        resolution->NewOption("800x480");
        if(!resolution->Select(current))
        {
            resolution->Select("1280x768");
        }
    }
    else if(ratio == "4:3")
    {
        resolution->NewOption("2048x1536");
        resolution->NewOption("1600x1200");
        resolution->NewOption("1400x1050");
        resolution->NewOption("1280x960");
        resolution->NewOption("1024x768");
        resolution->NewOption("800x600");
        resolution->NewOption("768x576");
        resolution->NewOption("640x480");
        resolution->NewOption("320x240");
        if(!resolution->Select(current))
        {
            resolution->Select("1024x768");
        }
    }
    else if(ratio == "3:2")
    {
        resolution->NewOption("1440x960");
        resolution->NewOption("1280x854");
        resolution->NewOption("1152x768");
        resolution->NewOption("720x480");
        if(!resolution->Select(current))
        {
            resolution->Select("1280x854");
        }
    }
}

void pawsLauncherWindow::LoadSettings()
{
    csConfigFile configPSC("/planeshift/psclient.cfg", psLaunchGUI->GetVFS());

    // Graphics Preset
    pawsComboBox* graphicsPreset = (pawsComboBox*)FindWidget("GraphicsPreset");
    graphicsPreset->Clear();
    graphicsPreset->NewOption("Highest");
    graphicsPreset->NewOption("High");
    graphicsPreset->NewOption("Medium");
    graphicsPreset->NewOption("Low");
    graphicsPreset->NewOption("Lowest");
    graphicsPreset->NewOption("Custom");

    csString setting = configUser->GetStr("PlaneShift.Graphics.Preset");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("PlaneShift.Graphics.Preset", "Custom");
    }
    graphicsPreset->Select(setting);

    // Aspect Ratio and Screen Resolution
    pawsComboBox* aspect = (pawsComboBox*)FindWidget("AspectRatio");
    aspect->Clear();
    aspect->NewOption("17:9");
    aspect->NewOption("16:10");
    aspect->NewOption("16:9");
    aspect->NewOption("5:4");
    aspect->NewOption("5:3");
    aspect->NewOption("4:3");
    aspect->NewOption("3:2");

    setting = configUser->GetStr("Video.AspectRatio");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("Video.AspectRatio");
    }
    aspect->Select(setting);
    HandleAspectRatio(setting);

    setting = configUser->GetStr("Video.ScreenWidth");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("Video.ScreenWidth");
    }

    csString height = configUser->GetStr("Video.ScreenHeight");
    if(height.Compare(""))
    {
        height = configPSC.GetStr("Video.ScreenHeight");
    }
    setting.AppendFmt("x%s", height.GetData());

    resolution->Select(setting);

    // Full screen
    pawsCheckBox* fullscreen = (pawsCheckBox*)FindWidget("Fullscreen");
    fullscreen->SetState(configUser->GetBool("Video.FullScreen"));

    // Sound
    pawsCheckBox* enableSound = (pawsCheckBox*)FindWidget("EnableSound");
    setting = configUser->GetStr("SndSys.Driver");
    enableSound->SetState(setting.Compare(""));

    pawsComboBox* soundRenderer = (pawsComboBox*)FindWidget("SoundRenderer");
    soundRenderer->NewOption("OpenAL");
    soundRenderer->NewOption("Software");

    setting = configUser->GetStr("System.PlugIns.iSndSysRenderer");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("System.PlugIns.iSndSysRenderer");
    }
    if(setting.Compare("crystalspace.sndsys.renderer.openal"))
    {
        soundRenderer->Select("OpenAL");
    }
    else
    {
        soundRenderer->Select("Software");
    }

    // Graphics
    pawsComboBox* screenDepth = (pawsComboBox*)FindWidget("ScreenDepth");
    screenDepth->NewOption("16");
    screenDepth->NewOption("32");

    setting = configUser->GetStr("Video.ScreenDepth");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("Video.ScreenDepth");
    }
    screenDepth->Select(setting);

    // AA
    pawsComboBox* antiAliasing = (pawsComboBox*)FindWidget("AntiAiasing");
    antiAliasing->Clear();
    antiAliasing->NewOption("0x");
    antiAliasing->NewOption("2x");
    antiAliasing->NewOption("2xQ");
    antiAliasing->NewOption("4x");
    antiAliasing->NewOption("4xQ");
    antiAliasing->NewOption("8x");
    antiAliasing->NewOption("8xQ");
    antiAliasing->NewOption("16x");
    antiAliasing->NewOption("16xQ");

    setting = configUser->GetStr("Video.OpenGL.MultiSamples");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("Video.OpenGL.MultiSamples");
    }
    bool quality;
    if(configUser->KeyExists("Video.OpenGL.MultisampleFavorQuality"))
    {
        quality = configUser->GetBool("Video.OpenGL.MultisampleFavorQuality");
    }
    else
    {
        quality = configPSC.GetBool("Video.OpenGL.MultisampleFavorQuality");
    }
    if(quality && !setting.Compare('0'))
    {
        setting.Append("xQ");
    }
    else
    {
        setting.Append("x");
    }
    antiAliasing->Select(setting);

    // Anistropic
    pawsComboBox* anistopicFiltering = (pawsComboBox*)FindWidget("AnisotropicFiltering");
    anistopicFiltering->Clear();
    anistopicFiltering->NewOption("0x");
    anistopicFiltering->NewOption("1x");
    anistopicFiltering->NewOption("2x");
    anistopicFiltering->NewOption("4x");
    anistopicFiltering->NewOption("8x");
    anistopicFiltering->NewOption("16x");

    setting = configUser->GetStr("Video.OpenGL.TextureFilterAnisotropy");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("Video.OpenGL.TextureFilterAnisotropy");
    }
    setting.Append("x");
    anistopicFiltering->Select(setting);

    // Texture Quality
    pawsComboBox* textureQuality = (pawsComboBox*)FindWidget("TextureQuality");
    textureQuality->Clear();
    textureQuality->NewOption("High");
    textureQuality->NewOption("Medium");
    textureQuality->NewOption("Low");
    textureQuality->NewOption("Very Low");

    int ds = configUser->GetInt("Video.OpenGL.TextureDownsample", (int)-1);
    if(ds == (int)-1)
    {
        ds = configPSC.GetInt("Video.OpenGL.TextureDownsample", 0);
    }
    switch(ds)
    {
    case 0:
        {
            setting = "High";
            break;
        }
    case 1:
        {
            setting = "Medium";
            break;
        }
    case 2:
        {
            setting = "Low";
            break;
        }
    case 4:
        {
            setting = "Very Low";
            break;
        }
    }
    textureQuality->Select(setting);

    pawsComboBox* shaders = (pawsComboBox*)FindWidget("Shaders");
    shaders->Clear();
    shaders->NewOption("Basic");
    shaders->NewOption("Advanced");

    setting = configUser->GetStr("PlaneShift.Graphics.Shaders");
    if(setting.Compare(""))
    {
        setting = configPSC.GetStr("PlaneShift.Graphics.Shaders");
    }
    shaders->Select(setting);

    pawsCheckBox* enableGrass = (pawsCheckBox*)FindWidget("EnableGrass");
    if(configUser->KeyExists("PlaneShift.Graphics.EnableGrass"))
    {
        enableGrass->SetState(configUser->GetBool("PlaneShift.Graphics.EnableGrass"));
    }
    else
    {
        enableGrass->SetState(configPSC.GetBool("PlaneShift.Graphics.EnableGrass"));
    }

    pawsCheckBox* VBO = (pawsCheckBox*)FindWidget("VBO");
    if(configUser->KeyExists("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object"))
    {
        VBO->SetState(configUser->GetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object"));
    }
    else
    {
        VBO->SetState(configPSC.GetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object"));
    }

    pawsCheckBox* loadAllMaps = (pawsCheckBox*)FindWidget("LoadAllMaps");
    if(configUser->KeyExists("Planeshift.Loading.AllMaps"))
    {
        loadAllMaps->SetState(configUser->GetBool("Planeshift.Loading.AllMaps"));
    }
    else
    {
        loadAllMaps->SetState(configPSC.GetBool("Planeshift.Loading.AllMaps"));
    }

    pawsCheckBox* keepMapsLoaded = (pawsCheckBox*)FindWidget("KeepMapsLoaded");
    if(configUser->KeyExists("Planeshift.Loading.KeepMaps"))
    {
        keepMapsLoaded->SetState(configUser->GetBool("Planeshift.Loading.KeepMaps"));
    }
    else
    {
        keepMapsLoaded->SetState(configPSC.GetBool("Planeshift.Loading.KeepMaps"));
    }

    pawsCheckBox* preloadModels = (pawsCheckBox*)FindWidget("PreloadModels");
    if(configUser->KeyExists("Planeshift.Loading.PreloadModels"))
    {
        preloadModels->SetState(configUser->GetBool("Planeshift.Loading.PreloadModels"));
    }
    else
    {
        preloadModels->SetState(configPSC.GetBool("Planeshift.Loading.PreloadModels"));
    }

    pawsCheckBox* threadedLoading = (pawsCheckBox*)FindWidget("ThreadedLoading");
    if(configUser->KeyExists("ThreadManager.AlwaysRunNow"))
    {
        threadedLoading->SetState(configUser->GetBool("ThreadManager.AlwaysRunNow"));
    }
    else
    {
        threadedLoading->SetState(configPSC.GetBool("ThreadManager.AlwaysRunNow"));
    }

    // Fill the skins
    pawsComboBox* skins = (pawsComboBox*)FindWidget("Skins");
    skins->Clear();
    csString skinPath = configPSC.GetStr("PlaneShift.GUI.Skin.Dir");
    csRef<iStringArray> files = psLaunchGUI->GetVFS()->FindFiles(skinPath);
    for (size_t i = 0; i < files->GetSize(); i++)
    {
        csString file = files->Get(i);
        file = file.Slice(skinPath.Length(),file.Length()-skinPath.Length());

        size_t dot = file.FindLast('.');
        csString ext = file.Slice(dot+1,3);
        if (ext == "zip")
        {
            skins->NewOption(file.Slice(0,dot));
        }
    }

    // Load the current skin
    csString skin = configUser->GetStr("PlaneShift.GUI.Skin.Selected");	
    if(!strcmp(skin,""))
    {
        // Try loading the default skin.
        skin = configPSC.GetStr("PlaneShift.GUI.Skin.Selected");
        if(!strcmp(skin,""))
        {
            // No skin selected.. shouldn't happen but it's not fatal.
            return;
        }
    }

    LoadSkin(skin);
}

void pawsLauncherWindow::SaveSettings()
{
    // Graphics Preset
    pawsComboBox* graphicsPreset = (pawsComboBox*)FindWidget("GraphicsPreset");
    configUser->SetStr("PlaneShift.Graphics.Preset", graphicsPreset->GetSelectedRowString());

    switch(graphicsPreset->GetSelectedRowNum())
    {
    case HIGHEST:
        {
            configUser->SetInt("Video.ScreenDepth", 32);
            configUser->SetInt("Video.OpenGL.MultiSamples", 16);
            configUser->SetInt("Video.OpenGL.TextureFilterAnisotropy", 16);
            configUser->SetInt("Video.OpenGL.TextureDownsample", 0);
            configUser->SetStr("PlaneShift.Graphics.Shaders", "Advanced");
            configUser->SetBool("PlaneShift.Graphics.EnableGrass", true);
            configUser->SetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object", true);
            configUser->SetBool("Planeshift.Loading.AllMaps", false);
            configUser->SetBool("Planeshift.Loading.KeepMaps", true);
            configUser->SetBool("Planeshift.Loading.PreloadModels", true);
            configUser->SetBool("ThreadManager.AlwaysRunNow", false);
            break;
        }
    case HIGH:
        {
            configUser->SetInt("Video.ScreenDepth", 32);
            configUser->SetInt("Video.OpenGL.MultiSamples", 4);
            configUser->SetInt("Video.OpenGL.TextureFilterAnisotropy", 8);
            configUser->SetInt("Video.OpenGL.TextureDownsample", 0);
            configUser->SetStr("PlaneShift.Graphics.Shaders", "Advanced");
            configUser->SetBool("PlaneShift.Graphics.EnableGrass", true);
            configUser->SetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object", true);
            configUser->SetBool("Planeshift.Loading.AllMaps", false);
            configUser->SetBool("Planeshift.Loading.KeepMaps", false);
            configUser->SetBool("Planeshift.Loading.PreloadModels", true);
            configUser->SetBool("ThreadManager.AlwaysRunNow", false);
            break;
        }
    case MEDIUM:
        {
            configUser->SetInt("Video.ScreenDepth", 32);
            configUser->SetInt("Video.OpenGL.MultiSamples", 2);
            configUser->SetInt("Video.OpenGL.TextureFilterAnisotropy", 4);
            configUser->SetInt("Video.OpenGL.TextureDownsample", 0);
            configUser->SetStr("PlaneShift.Graphics.Shaders", "Advanced");
            configUser->SetBool("PlaneShift.Graphics.EnableGrass", true);
            configUser->SetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object", true);
            configUser->SetBool("Planeshift.Loading.AllMaps", false);
            configUser->SetBool("Planeshift.Loading.KeepMaps", false);
            configUser->SetBool("Planeshift.Loading.PreloadModels", false);
            configUser->SetBool("ThreadManager.AlwaysRunNow", false);
            break;
        }
    case LOW:
        {
            configUser->SetInt("Video.ScreenDepth", 32);
            configUser->SetInt("Video.OpenGL.MultiSamples", 0);
            configUser->SetInt("Video.OpenGL.TextureFilterAnisotropy", 0);
            configUser->SetInt("Video.OpenGL.TextureDownsample", 2);
            configUser->SetStr("PlaneShift.Graphics.Shaders", "Basic");
            configUser->SetBool("PlaneShift.Graphics.EnableGrass", false);
            configUser->SetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object", true);
            configUser->SetBool("Planeshift.Loading.AllMaps", false);
            configUser->SetBool("Planeshift.Loading.KeepMaps", false);
            configUser->SetBool("Planeshift.Loading.PreloadModels", false);
            configUser->SetBool("ThreadManager.AlwaysRunNow", false);
            break;
        }
    case LOWEST:
        {
            configUser->SetInt("Video.ScreenDepth", 16);
            configUser->SetInt("Video.OpenGL.MultiSamples", 0);
            configUser->SetInt("Video.OpenGL.TextureFilterAnisotropy", 0);
            configUser->SetInt("Video.OpenGL.TextureDownsample", 4);
            configUser->SetStr("PlaneShift.Graphics.Shaders", "Basic");
            configUser->SetBool("PlaneShift.Graphics.EnableGrass", false);
            configUser->SetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object", true);
            configUser->SetBool("Planeshift.Loading.AllMaps", false);
            configUser->SetBool("Planeshift.Loading.KeepMaps", false);
            configUser->SetBool("Planeshift.Loading.PreloadModels", false);
            configUser->SetBool("ThreadManager.AlwaysRunNow", false);
            break;
        }
    case CUSTOM:
        {
            pawsComboBox* screenDepth = (pawsComboBox*)FindWidget("ScreenDepth");
            configUser->SetStr("Video.ScreenDepth", screenDepth->GetSelectedRowString());

            pawsComboBox* antiAliasing = (pawsComboBox*)FindWidget("AntiAiasing");
            csString selected = antiAliasing->GetSelectedRowString();
            configUser->SetStr("Video.OpenGL.MultiSamples", selected.Slice(0, selected.FindFirst("x")));

            pawsComboBox* anistopicFiltering = (pawsComboBox*)FindWidget("AnisotropicFiltering");
            selected = anistopicFiltering->GetSelectedRowString();
            configUser->SetStr("Video.OpenGL.TextureFilterAnisotropy", selected.Slice(0, selected.FindFirst("x")));

            pawsComboBox* textureQuality = (pawsComboBox*)FindWidget("TextureQuality");
            int value = 0;
            if(textureQuality->GetSelectedRowString().Compare("Medium"))
            {
                value = 1;
            }
            else if(textureQuality->GetSelectedRowString().Compare("Low"))
            {
                value = 2;
            }
            else if(textureQuality->GetSelectedRowString().Compare("Lowest"))
            {
                value = 4;
            }
            configUser->SetInt("Video.OpenGL.TextureDownsample", value);

            pawsComboBox* shaders = (pawsComboBox*)FindWidget("Shaders");
            configUser->SetStr("PlaneShift.Graphics.Shaders", shaders->GetSelectedRowString());

            pawsCheckBox* enableGrass = (pawsCheckBox*)FindWidget("EnableGrass");
            configUser->SetBool("PlaneShift.Graphics.EnableGrass", enableGrass->GetState());

            pawsCheckBox* VBO = (pawsCheckBox*)FindWidget("VBO");
            configUser->SetBool("Video.OpenGL.UseExtension.GL_ARB_vertex_buffer_object", VBO->GetState());

            pawsCheckBox* loadAllMaps = (pawsCheckBox*)FindWidget("LoadAllMaps");
            configUser->SetBool("Planeshift.Loading.AllMaps", loadAllMaps->GetState());

            pawsCheckBox* keepMapsLoaded = (pawsCheckBox*)FindWidget("KeepMapsLoaded");
            configUser->SetBool("Planeshift.Loading.KeepMaps", keepMapsLoaded->GetState());

            pawsCheckBox* preloadModels = (pawsCheckBox*)FindWidget("PreloadModels");
            configUser->SetBool("Planeshift.Loading.PreloadModels", preloadModels->GetState());

            pawsCheckBox* threadedLoading = (pawsCheckBox*)FindWidget("ThreadedLoading");
            configUser->SetBool("ThreadManager.AlwaysRunNow", !threadedLoading->GetState());

            break;
        }
    };

    // Aspect Ratio and Screen Resolution
    pawsComboBox* aspect = (pawsComboBox*)FindWidget("AspectRatio");
    configUser->SetStr("Video.AspectRatio", aspect->GetSelectedRowString());
    csString res = resolution->GetSelectedRowString();
    configUser->SetStr("Video.ScreenWidth", res.Slice(0, res.FindFirst('x')));
    configUser->SetStr("Video.ScreenHeight", res.Slice(res.FindFirst('x')+1));

    // Full screen
    pawsCheckBox* fullscreen = (pawsCheckBox*)FindWidget("Fullscreen");
    configUser->SetBool("Video.FullScreen", fullscreen->GetState());

    // Sound
    pawsCheckBox* enableSound = (pawsCheckBox*)FindWidget("EnableSound");
    if(enableSound->GetState())
    {
        configUser->DeleteKey("SndSys.Driver");
    }
    else
    {
        configUser->SetStr("SndSys.Driver", "crystalspace.sndsys.software.driver.null");
    }

    pawsComboBox* soundRenderer = (pawsComboBox*)FindWidget("SoundRenderer");
    if(soundRenderer->GetSelectedRowString().Compare("OpenAL"))
    {
        configUser->SetStr("System.PlugIns.iSndSysRenderer", "crystalspace.sndsys.renderer.openal");
    }
    else
    {
        configUser->SetStr("System.PlugIns.iSndSysRenderer", "crystalspace.sndsys.renderer.software");
    }

    pawsComboBox* skins = (pawsComboBox*)FindWidget("Skins");
    configUser->SetStr("PlaneShift.GUI.Skin.Selected", skins->GetSelectedRowString());

    // Save everything
    configUser->Save();
}

void pawsLauncherWindow::LoadSkin(const char* name)
{
    pawsComboBox* skins = (pawsComboBox*)FindWidget("Skins");
    pawsMultiLineTextBox* desc = (pawsMultiLineTextBox*)FindWidget("SkinDescription");
    pawsWidget* preview = FindWidget("SkinPreview");
    pawsButton* previewBtn = (pawsButton*)FindWidget("PreviewButton");
    pawsCheckBox* previewBox = (pawsCheckBox*)FindWidget("PreviewBox");

    csConfigFile configPSC("/planeshift/psclient.cfg", psLaunchGUI->GetVFS());
    csString skinPath = configPSC.GetStr("PlaneShift.GUI.Skin.Dir");

	// Create full path to skin.
    csString zip = skinPath + name + ".zip";

    // This .zip could be a file or a dir
    csString slash(CS_PATH_SEPARATOR);
    if (psLaunchGUI->GetVFS()->Exists(zip + slash))
        zip += slash;

    if (!psLaunchGUI->GetVFS()->Exists(zip))
    {
        printf("Current skin %s doesn't exist, skipping..\n", zip.GetData());
        return;
    }

    // Make sure the skin is selected
    if(skins->GetSelectedRowString() != name)
    {
        if(!skins->Select(name))
        {
            return;
        }
    }

    // Get the path
    csRef<iDataBuffer> real = psLaunchGUI->GetVFS()->GetRealPath(zip);
    
    // Mount the skin
    psLaunchGUI->GetVFS()->Unmount("/skin", mountedPath);
    psLaunchGUI->GetVFS()->Mount("/skin", real->GetData());
    mountedPath = real->GetData();

    // Parse XML
    csRef<iDocument> xml = ParseFile(PawsManager::GetSingleton().GetObjectRegistry(),"/skin/skin.xml");
    if(!xml)
    {
        Error1("Parse error in /skin/skin.xml");
        return;
    }
    csRef<iDocumentNode> root = xml->GetRoot();
    if(!root)
    {
        Error1("No XML root in /skin/skin.xml");
        return;
    }
    root = root->GetNode("xml");
    if(!root)
    {
        Error1("No <xml> tag in /skin/skin.xml");
        return;
    }
    csRef<iDocumentNode> skinInfo = root->GetNode("skin_information");
    if(!skinInfo)
    {
        Error1("No <skin_information> in /skin/skin.xml");
        return;
    }

    // Read XML
    csRef<iDocumentNode> nameNode           = skinInfo->GetNode("name");
    csRef<iDocumentNode> authorNode         = skinInfo->GetNode("author");
    csRef<iDocumentNode> descriptionNode    = skinInfo->GetNode("description");
    csRef<iDocumentNode> mountNode          = root->GetNode("mount_path");

    bool success = true;

    if(!mountNode)
    {
        printf("skin.xml is missing mount_path!\n");
        success = false;
    }

    if(!authorNode)
    {
        printf("skin.xml is missing author!\n");
        success = false;
    }

    if(!descriptionNode)
    {
        printf("skin.xml is missing description!\n");
        success = false;
    }

    if(!nameNode)
    {
        printf("skin.xml is missing name!\n");
        success = false;
    }

    if(!success)
        return;


    // Move data to variables
    csString skinname,author,description;
    skinname = nameNode->GetContentsValue();
    author = authorNode->GetContentsValue();
    description = descriptionNode->GetContentsValue();

    currentSkin = name;

    // Print the info
    csString info;
    info.Format("%s\nAuthor: %s\n%s",skinname.GetData(),author.GetData(),description.GetData());
    desc->SetText(info);

    // Reset the backgrounds
    preview->RemoveTitle();
    preview->SetBackground("Blue Background");
    previewBtn->SetBackground("Blue Background");
    previewBox->SetImages("radiooff","radioon");

    // Load the skin
    success = success && LoadResource("Examine Background","skintest_bg",mountNode->GetContentsValue());
    success = success && LoadResource("Standard Button","skintest_btn",mountNode->GetContentsValue());
    success = success && LoadResource("Blue Title","skintest_title",mountNode->GetContentsValue());
    success = success && LoadResource("radiooff","skintest_roff",mountNode->GetContentsValue());
    success = success && LoadResource("radioon","skintest_ron",mountNode->GetContentsValue());
    success = success && LoadResource("quit","quit",mountNode->GetContentsValue());

    if(!success)
    {
        PawsManager::GetSingleton().CreateWarningBox("Couldn't load skin! Check the console for detailed output");
        return;
    }

    preview->SetTitle("Skin preview","skintest_title","center","true");
    preview->SetMaxAlpha(1);
    preview->SetBackground("skintest_bg");

    previewBtn->SetMaxAlpha(1);
    previewBtn->SetBackground("skintest_btn");

    previewBox->SetImages("skintest_roff","skintest_ron");
}

bool pawsLauncherWindow::LoadResource(const char* resource,const char* resname, const char* mountPath)
{
    // Remove the resource if it exists
    PawsManager::GetSingleton().GetTextureManager()->Remove(resname);

    // Open the image list
    csRef<iDocument> xml = ParseFile(PawsManager::GetSingleton().GetObjectRegistry(),"/skin/imagelist.xml");
    if(!xml)
    {
        Error1("Parse error in /skin/imagelist.xml");
        return false;
    }
    csRef<iDocumentNode> root = xml->GetRoot();
    if(!root)
    {
        Error1("No XML root in /skin/imagelist.xml");
        return false;
    }
    csRef<iDocumentNode> topNode = root->GetNode("image_list");
    if(!topNode)
    {
        Error1("No <image_list> in /skin/imagelist.xml");
        return false;
    }

    csRef<iDocumentNodeIterator> iter = topNode->GetNodes();

    // Find the resource
    csString filename;
    while ( iter->HasNext() )
    {
        csRef<iDocumentNode> node = iter->Next();

        if ( node->GetType() != CS_NODE_ELEMENT )
            continue;

        if ( strcmp( node->GetValue(), "image" ) == 0 )
        {
            if(!strcmp(node->GetAttributeValue( "resource" ),resource))
            {
                filename = node->GetAttributeValue( "file" );
                break;
            }
        }
    }

    if(!filename.Length())
    {
        printf("Couldn't locate resource %s in the current skin!\n",resource);
        return false;
    }


    // Skin uses /paws/skin which we can't use since the app is already using that
    // So we need to replace that with /skin which we can and do use
    filename.DeleteAt(0,strlen(mountPath));
    filename.Insert(0,"/skin/");

    if(!psLaunchGUI->GetVFS()->Exists(filename))
    {
        printf("Skin is missing the '%s' resource at file '%s'!\n",resource,filename.GetData());
        return false;
    }

    csRef<iPawsImage> img;
    img.AttachNew(new pawsImageDrawable(filename.GetData(), resname, false, csRect(), 0, 0, 0, 0));
    PawsManager::GetSingleton().GetTextureManager()->AddPawsImage(img);

    return true;
}

void pawsLauncherWindow::OnListAction(pawsListBox* widget, int status)
{
    switch(widget->GetID())
    {
    case SKINS:
        {
            pawsComboBox* skins = (pawsComboBox*)FindWidget("Skins");
            csString selected = skins->GetSelectedRowString();
            if(!selected.Compare(currentSkin))
            {
                LoadSkin(selected);
            }
            break;
        }
    case ASPECT_RATIO:
        {
            pawsComboBox* aspect = (pawsComboBox*)FindWidget("AspectRatio");
            HandleAspectRatio(aspect->GetSelectedRowString());
            break;
        }
    }
}
