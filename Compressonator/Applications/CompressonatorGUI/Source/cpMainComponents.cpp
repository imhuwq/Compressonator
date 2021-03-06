//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
//=====================================================================

#include "cpMainComponents.h"
#include "Version.h"

static signalMsgHandler static_msghandler;
int g_OpenGLMajorVersion = 0;
int g_OpenGLMinorVersion = 0;

#define STR_WELCOME_PAGE    "Welcome Page"

C_Application_Options          g_Application_Options;
acProgressDlg                 *g_pProgressDlg = NULL;
bool                           g_bCompressing = false;  // Set true when we are compressing project items

// Hooked onto Progress Dialog.
void OnCancel()
{
    g_bAbortCompression = true;
}

//=========================================================
cpMainComponents::cpMainComponents(QDockWidget *root_dock, QMainWindow *parent)
    : QMainWindow(parent)
{
    if (parent == NULL)
        m_parent = this;
    else
        m_parent        = parent;

    //============================================
    fileMenu                = NULL;
    helpMenu                = NULL;
    fileToolBar             = NULL;
    CompressionToolBar      = NULL;
    newProjectAct           = NULL;
    saveToBatchFileAct      = NULL;
    openAct                 = NULL;
    saveAct                 = NULL;
    saveAsAct               = NULL;
    saveImageAct           = NULL;
    openImageFileAct        = NULL;
    exportAct               = NULL;
    exitAct                 = NULL;
    userGuideAct            = NULL;
    gettingStartedAct       = NULL;
    aboutAct                = NULL;
    settingsAct             = NULL;
    aboutQtAct              = NULL;
    compressAct             = NULL;
    imagediffAct            = NULL;
    MIPGenAct               = NULL;
    deleteImageAct          = NULL;
    closeAllDocuments       = NULL;
    showOutputAct           = NULL;
    showWelcomePageAct      = NULL;
    windowMenu              = NULL;
    isCompressInProgress    = false;
    m_ForceImageRefresh     = false;

#ifdef ENABLE_AGS_SUPPORT
    onHDRButton = NULL;
    // AGS specific
    m_bIsHDRAvailableOnPrimary = false;
    m_bIsFullScreenModeOn   = false;
    m_agsContext = nullptr;
    m_DeviceIndex = 0;
    m_DisplayIndex = 0;
#endif

#ifdef USE_MAINA_IMAVEVIEW_TOOLBAR
    ImageViewToolBar        = NULL;
    imageview_zoomInAct     = NULL;
    imageview_zoomOutAct    = NULL;
    imageview_RedAct        = NULL;
    imageview_GreenAct      = NULL;
    imageview_BlueAct       = NULL;
    imageview_AlphaAct      = NULL;
    imageview_FitScreenAct  = NULL;
#endif

    m_welcomePage           = NULL;
    m_imagePropertyView     = NULL;
    m_genmips               = NULL;
    m_setcompressoptions    = NULL;
    m_projectview           = NULL;
    m_imageview             = NULL;
    m_imageCompare          = NULL;
    m_3Dmodelview           = NULL;
    m_activeImageTab        = NULL;
    app_welcomepage         = NULL;
    m_frame                 = NULL;

    m_numRecentFiles        = 0;
    m_projectsRecentFiles.clear();

    m_showAppSettingsDialog = true;
    m_viewDiff = false;
    m_sSettingsFile = "CompressSettings.ini";


    if (m_showAppSettingsDialog)
    {
        m_setapplicationoptions = new CSetApplicationOptions("Application Settings", this);
        m_setapplicationoptions->hide();
    }


    //============================================
    app_welcomepage = root_dock;

    m_parent->setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);


    //
    m_blankpage = new acCustomDockWidget("", this);
    m_blankpage->setTitleBarWidget(NULL);
    m_blankpage->m_fileName = "";
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    m_blankpage->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_parent->addDockWidget(Qt::RightDockWidgetArea, m_blankpage);
    if (m_blankpage->custTitleBar)
    {
        m_blankpage->custTitleBar->setButtonCloseEnabled(false);
        m_blankpage->custTitleBar->setButtonToolBarShow(false);
        m_blankpage->lower();
        //m_blankpage->setStyleSheet("QTabWidget::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }");
        // setTabEnabled(false);
    }

    // Status View when compressing
    m_CompressStatusDialog = new CompressStatusDialog("Output", this);
    setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    m_CompressStatusDialog->resize(300, 290);
    m_parent->addDockWidget(Qt::BottomDockWidgetArea, m_CompressStatusDialog);
    m_CompressStatusDialog->hideOutput();

    connect(&static_msghandler, SIGNAL(signalMessage(const char *)), this, SLOT(browserMsg(const char *)));

    m_projectview = new ProjectView("Project", m_CompressStatusDialog, this);
    m_projectview->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_projectview->resize(300, 500);
    m_projectview->setMaximumWidth(300);
    m_projectview->setMinimumWidth(200);
    m_parent->addDockWidget(Qt::LeftDockWidgetArea, m_projectview);

    QString tempSetting = m_sSettingsFile;
    QFileInfo fileInfo(tempSetting);
    if (!fileInfo.isWritable())
    {
        QFileInfo fileInfo2(m_projectview->m_curProjectFilePathName);
        m_sSettingsFile = fileInfo2.dir().path();
        m_sSettingsFile.append(QDir::separator());
        m_sSettingsFile.append(tempSetting);
        m_sSettingsFile.replace("/", "\\");
    }
    //
    m_imagePropertyView = new CImagePropertyView("  Properties", this);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    m_imagePropertyView->resize(300, 290);
    m_parent->addDockWidget(Qt::LeftDockWidgetArea, m_imagePropertyView);


    m_welcomePage = new CWelcomePage(STR_WELCOME_PAGE, this);
    m_welcomePage->m_fileName = "";
    m_parent->tabifyDockWidget(m_blankpage, m_welcomePage);
    m_welcomePage->resize(600,400);
    if (m_welcomePage->custTitleBar)
        m_welcomePage->custTitleBar->setTitle(STR_WELCOME_PAGE);
    m_welcomePage->setAllowedAreas(Qt::RightDockWidgetArea);
    connect(m_welcomePage, SIGNAL(WebPageButtonClick(QString &, QString &)), this, SLOT(OnWelcomePageButtonClick(QString &, QString &)));

    // Get the blank page tab and set disable property to hide it
    QTabBar *tabBar = this->findChild<QTabBar *>();
    if (tabBar)
    {
        tabBar->setStyleSheet("QTabBar::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }");
        tabBar->setTabEnabled(0, false);
    }

    m_setcompressoptions = new CSetCompressOptions("Destination Setting", this);
    m_setcompressoptions->hide();

    connect(m_projectview, SIGNAL(ViewImageFile(QString &, QTreeWidgetItem *)), this, SLOT(AddImageView(QString &, QTreeWidgetItem *)));
    connect(m_projectview, SIGNAL(ViewImageFileDiff(C_Destination_Options *, QString &, QString &)), this, SLOT(AddImageDiff(C_Destination_Options *, QString &, QString &)));
    connect(m_projectview, SIGNAL(View3DModelFileDiff(C_3D_Source_Info *, QString &, QString &)), this, SLOT(Add3DModelDiff(C_3D_Source_Info *, QString &, QString &)));
    connect(m_projectview, SIGNAL(DeleteImageView(QString &)), this, SLOT(OnDeleteImageView(QString &)));
    connect(m_projectview, SIGNAL(UpdateData(QObject *)), m_imagePropertyView, SLOT(OnUpdateData(QObject *)));
    connect(m_projectview, SIGNAL(AddCompressSettings(QTreeWidgetItem *)), this, SLOT(OnAddCompressSettings(QTreeWidgetItem *)));
    connect(m_projectview, SIGNAL(EditCompressSettings(QTreeWidgetItem *)), this, SLOT(onEditCompressSettings(QTreeWidgetItem *)));

    connect(m_projectview, SIGNAL(OnAddedCompressSettingNode()), this, SLOT(onAddedCompressSettingNode()));


    connect(m_projectview, SIGNAL(OnCompressionStart()), this, SLOT(onCompressionStart()));
    connect(m_projectview, SIGNAL(OnCompressionDone()),  this, SLOT(onCompressionDone()));

    connect(m_projectview, SIGNAL(OnCompressionStart()), m_imagePropertyView, SLOT(onCompressionStart()));
    connect(m_projectview, SIGNAL(OnProcessing(QString &)), this, SLOT(onProcessing(QString &)));
    connect(m_projectview, SIGNAL(OnCompressionDone()), m_imagePropertyView, SLOT(onCompressionDone()));

    connect(m_projectview, SIGNAL(OnSourceImage(int)), this, SLOT(onSourceImage(int)));
    connect(m_projectview, SIGNAL(OnDecompressImage()) , this, SLOT(onDecompressImage()));

    connect(m_projectview, SIGNAL(OnSourceImage(int)), m_imagePropertyView, SLOT(onSourceImage(int)));
    connect(m_projectview, SIGNAL(OnProjectLoaded(int)), this, SLOT(onProjectLoaded(int)));


    connect(this, SIGNAL(OnImageLoadStart()), m_projectview, SLOT(onImageLoadStart()));
    connect(this, SIGNAL(OnImageLoadDone()),  m_projectview, SLOT(onImageLoadDone()));
    connect(this, SIGNAL(OnImageLoadStart()), m_imagePropertyView, SLOT(onImageLoadStart()));
    connect(this, SIGNAL(OnImageLoadDone()),  m_imagePropertyView, SLOT(onImageLoadDone()));



#ifdef USE_MSGHANDLER
    connect(&static_msghandler, SIGNAL(signalMessage(const char *)), m_projectview, SLOT(OnGlobalMessage(const char *)));
#endif

    connect(this, SIGNAL(SetCurrentItem(QString &)), m_projectview, SLOT(onSetCurrentItem(QString &)));
    connect(m_setcompressoptions, SIGNAL(SaveCompressSettings(QTreeWidgetItem *, C_Destination_Options &)), this, SLOT(AddImageCompSettings(QTreeWidgetItem *, C_Destination_Options &)));

    // QRect scr = QApplication::desktop()->screenGeometry();
    // 
    m_genmips = new CGenMips("Generate MIP Maps",NULL);
    m_genmips->hide();
    connect(m_genmips, SIGNAL(generateMIPMap(int, QTreeWidgetItem *)), this, SLOT(onGenerateMIPMap(int, QTreeWidgetItem *)));
        
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    
    readSettings();

    // Set some global setting 
    #ifdef  ENABLED_USER_GPUVIEW
    g_useCPUDecode = (g_Application_Options.m_ImageViewDecode == g_Application_Options.ImageDecodeWith::CPU);
    #else
    g_useCPUDecode = true;
    #endif
#ifdef USE_COMPUTE
    g_useCPUEncode = g_Application_Options.m_ImageEncode == g_Application_Options.ImageEncodeWith::CPU;
#endif
    setUnifiedTitleAndToolBarOnMac(true);

    if (m_showAppSettingsDialog)
    {
        // Act on read settings for application startup
        if (g_Application_Options.m_loadRecentFile)
        {
            if (m_numRecentFiles > 0)
            {
                recentFileActs[0]->trigger();
            }
        }
    }

    m_welcomePage->GoToAMDHomePage(m_projectsRecentFiles);

    //=============================
    // Help About
    //=============================
    m_pacHelpAboutDialog = new CHelpAboutDialog(this);
    // Sett current project name on app title bar
    SetProjectWindowTitle();

    // Get the product version:
    m_apptitle = "Compressonator";
   
    QString ver  = QString("%1.%2.%3.%4").arg(
        QString::number(VERSION_MAJOR_MAJOR),
        QString::number(VERSION_MAJOR_MINOR),
        QString::number(VERSION_MINOR_MAJOR),
        QString::number(0)
        );

    // Compression Connections

    connect(m_imagePropertyView, SIGNAL(saveSetting(QString *)), this, SLOT(onPropertyViewSaveSetting(QString *)));
    connect(m_imagePropertyView, SIGNAL(compressImage(QString *)), this, SLOT(onPropertyViewCompressImage(QString *)));

#ifdef ENABLE_AGS_SUPPORT
    // Get AGS Settings
    AGSGetDisplayInfo(&m_settings);

    if (m_bIsHDRAvailableOnPrimary)
        onHDRButton->setText("Full Screen with HDR");
    else
        onHDRButton->setText("Full Screen");
#endif

    // Adding a global Progress Dialog,
    // This replaces the one defined in ProjectView
    // Progress Dialog During Compression
    g_pProgressDlg = new acProgressDlg(this);
    g_pProgressDlg->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    g_pProgressDlg->ShowCancelButton(true, &OnCancel);
    g_pProgressDlg->SetHeader("");
    g_pProgressDlg->SetLabelText("");
    g_pProgressDlg->SetRange(0, 100);
    g_pProgressDlg->hide();

}

void cpMainComponents::SetProjectWindowTitle()
{
    if (m_projectview)
    {
        setWindowTitle(m_projectview->m_curProjectName);
    }
}

void cpMainComponents::OnWelcomePageButtonClick(QString &Request, QString &Msg)
{
    #define PROJECT_DIR "/Projects/"

    if (Request.compare("new_project") == 0)
    {
        openNewProject();
    }
    else
    // qDebug() << Request << " Msg: " << Msg;
    if (Request.compare("open_project") == 0)
    {

        if (!m_projectview) return;
        if (!m_projectview->userSaveProjectAndContinue()) return;


        bool found = false;
        if(Msg.indexOf(".cprj")==-1)
            Msg.append(PROJECT_EXTENSION);

        // Try the Current Path
        if (!found)
        {
            QString ProjectFile = QDir::currentPath();
            ProjectFile.append(PROJECT_DIR);
            ProjectFile.append(Msg);
            QFile Fout(ProjectFile);
            if (Fout.exists())
            {
                m_projectview->loadProjectFile(ProjectFile);
                found = true;
            }
        }

        // Try the Application Dir
        if (!found)
        {
            // First Try looking for the project in sample folder
            QString ProjectFile = qApp->applicationDirPath();
            ProjectFile.append(PROJECT_DIR);
            ProjectFile.append(Msg);
            QFile Fout(ProjectFile);
            if (Fout.exists())
            {
                m_projectview->loadProjectFile(ProjectFile);
                found = true;
            }
        }

        // Try the Working Dir
        if (!found)
        {
            // This is windows specific!
            QString pwd("");
            char * ENV;
            // Check if user set our envniornment var
            ENV = getenv(ENV_COMPRESSONATOR_ROOT);
            if (ENV)
                pwd.append(ENV);
            else
            {   // check Pathname of the current working dir
                ENV = getenv("PWD");
            }
            if (ENV)
            {
                pwd.append(PROJECT_DIR);
                pwd.append(Msg);
                QFile Fout(pwd);
                if (Fout.exists())
                {
                    m_projectview->loadProjectFile(pwd);
                    found = true;
                }
            }
        }

        // Try Recent Files
        if (!found)
        {
            for (int i = 0; i < m_numRecentFiles; i++)
            {
                // The list is in &%1 %2 format so the first 3 char are removed
                // Our max recents is 5 which is 1 char size
                QString proj = recentFileActs[i]->text();
                proj.remove(0, 3);
                if (Msg.compare(proj) == 0)
                {
                    recentFileActs[i]->trigger();
                    found = true;
                    break;
                }
            }
        }

        if (found)
            setCurrentFile(m_projectview->m_curProjectFilePathName);
    }
    else
    if (Request.compare("show_quick_start") == 0)
    {
        OpenCHMFile(COMPRESSONATOR_GETTING_STARTED);
    }
    else
    if (Request.compare("show_help") == 0)
    {
        OpenCHMFile(COMPRESSONATOR_USER_GUIDE);
    }
}

void cpMainComponents::closeEvent(QCloseEvent *event)
{
    if (g_bCompressing)
    {
        g_bAbortCompression = true;
        // loop until all compression codecs abort
        int maxwait = 3000; // > 3 seconds
        while (g_bCompressing)
        {
            Sleep(1);
            maxwait--;
            if (maxwait == 0) break;
            QApplication::processEvents();
        }
    }
    CMP_ShutdownDecompessLibrary();

    if (m_projectview)
    {
        if (!m_projectview->userSaveProjectAndContinue())
        {
            event->ignore();
            return;
        }
        setCurrentFile(m_projectview->m_curProjectFilePathName);
        m_projectview->clearProjectTreeView();
    }

   writeSettings();
   
   qApp->quit();
   event->accept();
}

void cpMainComponents::openProjectFile()
{
    if (m_projectview)
    {
        m_projectview->openProjectFile();
        setCurrentFile(m_projectview->m_curProjectFilePathName);
    }
}

void cpMainComponents::openNewProject()
{
    if (m_projectview)
    {
        m_projectview->openNewProjectFile();
        setCurrentFile(m_projectview->m_curProjectFilePathName);
    }
}

bool cpMainComponents::saveProjectToBatchFile()
{
    if (m_projectview)
    {
        m_projectview->saveToBatchFile();
    }
    return true;
}

void cpMainComponents::openImageFile()
{
    if (m_projectview)
    {
        m_projectview->OpenImageFile();
    }
}

void cpMainComponents::imageDiff()
{
    if (m_projectview)
    {
        m_projectview->diffImageFiles();
    }
}

void cpMainComponents::deleteImageFile()
{
    if (m_projectview)
    {
        m_projectview->UserDeleteItems();
    }
}

bool cpMainComponents::saveProjectFile()
{
    if (m_projectview)
    {
        m_projectview->saveProjectFile();
        setCurrentFile(m_projectview->m_curProjectFilePathName);
    }
    return true;
}

bool cpMainComponents::saveAsProjectFile()
{
    if (m_projectview)
    {
        m_projectview->saveAsProjectFile();
        setCurrentFile(m_projectview->m_curProjectFilePathName);
    }
    return true;
}

bool cpMainComponents::saveImageFile()
{
    if (m_projectview)
    {
        m_projectview->saveImageAs();
        setCurrentFile(m_projectview->m_curProjectFilePathName);
    }
    return true;
}

void cpMainComponents::settings()
{
    if (m_showAppSettingsDialog)
    {
        if (m_setapplicationoptions)
        {
            m_setapplicationoptions->UpdateViewData();
            m_setapplicationoptions->show();
            m_setapplicationoptions->raise();
        }
    }
}

void cpMainComponents::about()
{
    if (m_pacHelpAboutDialog)
        m_pacHelpAboutDialog->show();
}

void cpMainComponents::onShowWelcomePage()
{
    if (m_welcomePage)
    {
        m_welcomePage->show();
        m_welcomePage->raise();
    }
}

void cpMainComponents::onShowOutput()
{
   if (m_CompressStatusDialog)
   {
       m_CompressStatusDialog->showOutput();
       m_CompressStatusDialog->raise();
   }
}

void cpMainComponents::onCloseAllDocuments()
{
    QList<acCustomDockWidget *> dockWidgets = m_parent->findChildren<acCustomDockWidget *>();
    QListIterator<acCustomDockWidget *> iter(dockWidgets);
    acCustomDockWidget *dock;
    int i = 0;
    while (iter.hasNext())
    {
        dock = iter.next();
        i++;
        if (dock)
        {
            QString FileName = dock->m_fileName;
            if (FileName.size() > 0)
            {
                dock->close();
                if (dock)
                {
                    if (dock->custTitleBar)
                    {
                        // we have a image diff with 3 other sub classes of type acCustomDockWidget
                        // that will be deleted.
                        if (dock->custTitleBar->m_close)
                        {
                            if (iter.hasNext())
                                iter.next();
                            if (iter.hasNext())
                                iter.next();
                            if (iter.hasNext())
                                iter.next();
                        }
                    }
                    delete dock;
                    dock = NULL;
                }
            }
        }
    }
}

void cpMainComponents::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        if (m_projectview)
        {
            if (m_projectview->loadProjectFile(action->data().toString()))
            {
                curFile = m_projectview->m_curProjectFilePathName;
            }

        }

    }
}

void cpMainComponents::createActions()
{
    newProjectAct = new QAction(QIcon(":/CompressonatorGUI/Images/filenew.png"), tr("&New Project..."), this);
    if (newProjectAct)
    {
        newProjectAct->setShortcuts(QKeySequence::New);
        newProjectAct->setStatusTip(tr("Create a new project file"));
        connect(newProjectAct, SIGNAL(triggered()), this, SLOT(openNewProject()));
    }

    openAct = new QAction(QIcon(":/CompressonatorGUI/Images/open.png"), tr("&Open project..."), this);
    if (openAct)
    {
        openAct->setShortcuts(QKeySequence::Open);
        openAct->setStatusTip(tr("Open an existing project file"));
        connect(openAct, SIGNAL(triggered()), this, SLOT(openProjectFile()));
    }

    saveAct = new QAction(QIcon(":/CompressonatorGUI/Images/save.png"), tr("&Save project"), this);
    if (saveAct)
    {
        saveAct->setShortcuts(QKeySequence::Save);
        saveAct->setStatusTip(tr("Save project file"));
        connect(saveAct, SIGNAL(triggered()), this, SLOT(saveProjectFile()));
    }

    saveAsAct = new QAction(QIcon(""), tr("&Save project as..."), this);
    if (saveAsAct)
    {
        saveAsAct->setShortcuts(QKeySequence::SaveAs);
        saveAsAct->setStatusTip(tr("Save project as ..."));
        connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAsProjectFile()));
    }

    saveImageAct = new QAction(QIcon(""), tr("&Save image as..."), this);
    if (saveImageAct)
    {
        saveImageAct->setStatusTip(tr("Save image as ..."));
        connect(saveImageAct, SIGNAL(triggered()), this, SLOT(saveImageFile()));
        saveImageAct->setEnabled(false);
    }

    
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
            this, SLOT(openRecentFile()));
    }

    saveToBatchFileAct = new QAction(QIcon(""), tr("&Export to batch file..."), this);
    if (saveToBatchFileAct)
    {
        saveToBatchFileAct->setStatusTip(tr("Export the project file to a command line batch file"));
        connect(saveToBatchFileAct, SIGNAL(triggered()), this, SLOT(saveProjectToBatchFile()));
        saveToBatchFileAct->setEnabled(false);
    }

    openImageFileAct = new QAction(QIcon(":/CompressonatorGUI/Images/file.png"), tr("&Open Image File..."), this);
    if (openImageFileAct)
    {
        //ToDo :: openImageFileAct->setShortcuts();
        openImageFileAct->setStatusTip(tr("Open an image file"));
        connect(openImageFileAct, SIGNAL(triggered()), this, SLOT(openImageFile()));
    }

    deleteImageAct = new QAction(QIcon(":/CompressonatorGUI/Images/delete.png"), tr("&Delete current image"), this);
    if (deleteImageAct)
    {
        //ToDo :: deleteImageAct->setShortcuts();
        deleteImageAct->setStatusTip(tr("Delete selected image file"));
        connect(deleteImageAct, SIGNAL(triggered()), this, SLOT(deleteImageFile()));
        deleteImageAct->setEnabled(false);
    }

    compressAct = new QAction(QIcon(":/CompressonatorGUI/Images/compress.png"), tr("&Process selected images"), this);
    if (compressAct)
    {
        compressAct->setStatusTip(tr("Compress all selected items"));
        connect(compressAct, SIGNAL(triggered()), m_projectview, SLOT(OnStartCompression()));
        compressAct->setEnabled(true); //enabled by default
    }

    imagediffAct = new QAction(QIcon(":/CompressonatorGUI/Images/imagediff.png"), tr("&View Image Diff"), this);
    if (imagediffAct)
    {
        imagediffAct->setStatusTip(tr("View Image Diff"));
        connect(imagediffAct, SIGNAL(triggered()), this, SLOT(imageDiff()));
        imagediffAct->setEnabled(true);
    }


    MIPGenAct = new QAction(QIcon(":/CompressonatorGUI/Images/MIP.png"), tr("&Generate MIP maps for current source image"), this);
    if (MIPGenAct)
    {
        MIPGenAct->setStatusTip(tr("Generate MIP maps on current source image"));
        connect(MIPGenAct, SIGNAL(triggered()), this, SLOT(genMIPMaps()));
        MIPGenAct->setEnabled(true);
    }

#ifdef ENABLE_AGS_SUPPORT
    onHDRButton = new QPushButton("Full Screen", this);
    if (onHDRButton)
    {
        onHDRButton->setStatusTip(tr("Sets Full screen on or off , If available HDR is turned on in Full Screen"));
        connect(onHDRButton, SIGNAL(released()), this, SLOT(handleHDRon()));
    }
#endif

#ifdef USE_MAIN_IMAVEVIEW_TOOLBAR
    imageview_zoomInAct    = new QAction(QIcon(":/CompressonatorGUI/Images/ZoomIn.png"), tr("&Zoom into Image "), this);
    imageview_zoomOutAct   = new QAction(QIcon(":/CompressonatorGUI/Images/ZoomOut.png"), tr("&Zoom out of Image"), this);
    imageview_RedAct       = new QAction(QIcon(":/CompressonatorGUI/Images/redStone.png"), tr("Show or Hide  Red channel"), this);
    imageview_GreenAct     = new QAction(QIcon(":/CompressonatorGUI/Images/greenStone.png"), tr("Show or Hide Green channel"), this);
    imageview_BlueAct      = new QAction(QIcon(":/CompressonatorGUI/Images/blueStone.png"), tr("Show or Hide Blue channel"), this);
    imageview_AlphaAct     = new QAction(QIcon(":/CompressonatorGUI/Images/circle.png"), tr("Show or Hide Alpha channel"), this);
    imageview_FitScreenAct = new QAction(QIcon(":/CompressonatorGUI/Images/expand.png"), tr("&Fit in Window"), this);
#endif

    exitAct = new QAction(QIcon(""), tr("&Exit"), this);
    if (exitAct)
    {
        exitAct->setStatusTip(tr("Exit Application"));
        connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    }

    if (m_showAppSettingsDialog)
    {
        settingsAct = new QAction(QIcon(":/CompressonatorGUI/Images/Gear.png"), tr("&Set Application Options"), this);
        if (settingsAct)
        {
            settingsAct->setStatusTip(tr("Set Application Options"));
            connect(settingsAct, SIGNAL(triggered()), this, SLOT(settings()));
        }
    }

    showWelcomePageAct = new QAction(tr("Welcome Page"), this);
    if (showWelcomePageAct)
    {
        showWelcomePageAct->setStatusTip(tr("View Welcome Page"));
        connect(showWelcomePageAct, SIGNAL(triggered()), this, SLOT(onShowWelcomePage()));
    }

    showOutputAct = new QAction(tr("Output"), this);
    if (showOutputAct)
    {
        showOutputAct->setStatusTip(tr("View Output Window"));
        connect(showOutputAct, SIGNAL(triggered()), this, SLOT(onShowOutput()));
    }

    closeAllDocuments = new QAction(tr("Close all Image Views"), this);
    if (closeAllDocuments)
    {
        closeAllDocuments->setStatusTip(tr("Close all opened image views"));
        connect(closeAllDocuments, SIGNAL(triggered()), this, SLOT(onCloseAllDocuments()));
    }

    gettingStartedAct = new QAction(tr("Getting Started ..."), this);
    if (gettingStartedAct)
    {
        gettingStartedAct->setStatusTip(tr("Getting Started"));
        connect(gettingStartedAct, SIGNAL(triggered()), this, SLOT(gettingStarted()));
    }

    userGuideAct = new QAction(tr("User Guide ..."), this);
    if (userGuideAct)
    {
        userGuideAct->setStatusTip(tr("User Guide"));
        connect(userGuideAct, SIGNAL(triggered()), this, SLOT(userGuide()));
    }

    aboutAct = new QAction(tr("About Compressonator"), this);
    if (aboutAct)
    {
        aboutAct->setStatusTip(tr("About Compressonator"));
        connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    }
}

void cpMainComponents::onGenerateMIPMap(int nMinSize, QTreeWidgetItem *item)
{
    if (!m_projectview) return;

    if (item)
    {
        QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
        C_Source_Info *data = v.value<C_Source_Info *>();
    
        if (data->m_MipImages)
            if (data->m_MipImages->mipset)
                if (data->m_MipImages->mipset->m_compressed)
                {
                    if (m_CompressStatusDialog)
                    {
                        m_CompressStatusDialog->onClearText();
                        m_CompressStatusDialog->showOutput();
                    }
                    PrintInfo("Mipmap generation is not supported for compressed image."); 
                    return;
                }
    }
   
    if (nMinSize <= 0) nMinSize = 1;

    PluginInterface_Filters *plugin_Filter;
    plugin_Filter = reinterpret_cast<PluginInterface_Filters *>(g_pluginManager.GetPlugin("FILTER", "BOXFILTER"));
    if (plugin_Filter)
    {
        if (item)
        {
            QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
            C_Source_Info *data = v.value<C_Source_Info *>();
            if (data)
            {
                if (m_CompressStatusDialog)
                {
                    m_CompressStatusDialog->onClearText();
                    m_CompressStatusDialog->showOutput();
                }

                // Quick Check to see if lowest level is lower then current image size
                int min = data->m_Width;
                if (min > data->m_Height) min = data->m_Height;
                if (nMinSize < min)
                {

                    if (data->m_MipImages)
                        if (data->m_MipImages->mipset)
                        {
                            if (m_CompressStatusDialog)
                            {
                                m_CompressStatusDialog->onClearText();
                                m_CompressStatusDialog->show();
                            }

                            // Generate the MIP levels
                            plugin_Filter->TC_GenerateMIPLevels(data->m_MipImages->mipset, nMinSize);

                            // Create Image views for the levels
                            CImageLoader ImageLoader;
                            ImageLoader.UpdateMIPMapImages(data->m_MipImages);

                            if (m_CompressStatusDialog)
                            {
                                QString msg = "<b>Generated : ";
                                msg.append(QString::number(data->m_MipImages->mipset->m_nMipLevels));
                                msg.append(" MIP level(s)</b>");
                                msg.append(" with a minimum size set to ");
                                msg.append(QString::number(nMinSize));
                                msg.append(" px");
                                m_CompressStatusDialog->appendText(msg);
                                m_ForceImageRefresh = true;
                            }

                            m_projectview->SignalUpdateData(item, TREETYPE_IMAGEFILE_DATA);
                            m_projectview->m_clicked_onIcon = true;
                            m_projectview->onTree_ItemClicked(item, 0);
                        }
                }
                else
                {
                    if (m_CompressStatusDialog)
                    {
                        m_CompressStatusDialog->appendText("No MIP levels generated: Please select a level lower than current image size");
                    }
                }
            }
            delete plugin_Filter;
        }
    }
}

void cpMainComponents::genMIPMaps()
{
    if (m_projectview)
    {
        QTreeWidgetItem *item = m_projectview->GetCurrentItem(TREETYPE_IMAGEFILE_DATA);
        if (item)
        {
            QString   Setting = item->text(0);
            QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
            C_Source_Info *data = v.value<C_Source_Info *>();
            if (data)
            {
                // regenrate mip map
                if (data->m_MipImages->mipset->m_nMipLevels > 1 || data->m_MipImages->Image_list.count()>1)
                {
                    int n = data->m_MipImages->Image_list.count();
                    for (int i = 1; i < n; i++)
                    {
                        data->m_MipImages->Image_list.removeLast();
                    }

                    data->m_MipImages->mipset->m_nMipLevels = 1;
                }

                m_genmips->m_mipsitem = item;
                m_genmips->setMipLevelDisplay(data->m_Width, data->m_Height);
                m_genmips->show();

                // Generate mipmap only once- no regenerate then uncomment code block below
                //if (data->m_MipImages->mipset->m_nMipLevels <= 1)
                //{
                //    m_genmips->setMipLevelDisplay(data->m_Width, data->m_Height);
                //    m_genmips->show();
                //}
                //else
                //{
                //    QMessageBox msgBox;
                //    msgBox.setText("The image already has MIP levels!");
                //    msgBox.setStandardButtons(QMessageBox::Ok);
                //    msgBox.exec();
                //}
            }
            else
            {
                if (m_projectview)
                {
                    QTreeWidgetItemIterator it(m_projectview->m_projectTreeView);
                    QString   Setting = (*it)->text(0);
                    ++it; //skip add image node
                    if (!(*it))
                    {
                        m_projectview->m_CompressStatusDialog->appendText("Please add the image file that you would like to generate mip map with.");
                        m_projectview->m_CompressStatusDialog->show();
                        return;
                    }
                    while (*it) 
                    {
                        QVariant v = (*it)->data(TREE_SourceInfo, Qt::UserRole);
                        QString   Setting = (*it)->text(0);
                        //if (levelType == TREETYPE_IMAGEFILE_DATA)
                        //{
                            C_Source_Info *data = v.value<C_Source_Info *>();
                            if (data)
                            {
                                // regenrate mip map
                                if (data->m_MipImages->mipset->m_nMipLevels > 1 || data->m_MipImages->Image_list.count()>1)
                                {
                                    int n = data->m_MipImages->Image_list.count();
                                    for (int i = 1; i < n; i++)
                                    {
                                        data->m_MipImages->Image_list.removeLast();
                                    }

                                    data->m_MipImages->mipset->m_nMipLevels = 1;
                                }

                                m_genmips->m_mipsitem = (*it);
                                m_genmips->setMipLevelDisplay(data->m_Width, data->m_Height);
                                m_genmips->show();
                            }
                        //}
                        if (*it)
                            ++it;
                        else
                            break;
                    }
                }
            }
        }
        else
        {
            if (m_projectview)
            {
                m_projectview->m_CompressStatusDialog->appendText("Please add the image file that you would like to generate mip map with.");
                m_projectview->m_CompressStatusDialog->show();
            }
        }
    }
}

#ifdef ENABLE_AGS_SUPPORT
//--------------------------------------------------------------------------------------
void cpMainComponents::AGSGetDisplayInfo(AGSDisplaySettings *settings)
{

    int displayIndex = 0;
    DISPLAY_DEVICEA displayDevice;
    displayDevice.cb = sizeof(displayDevice);
    while (EnumDisplayDevicesA(0, displayIndex, &displayDevice, 0))
    {
        displayIndex++;
    }

    AGSGPUInfo gpuInfo;

    AGSConfiguration config = {};
    config.crossfireMode = AGS_CROSSFIRE_MODE_EXPLICIT_AFR;

    if (agsInit(&m_agsContext, &config, &gpuInfo) == AGS_SUCCESS)
    {
        for (int gpuIndex = 0; gpuIndex < gpuInfo.numDevices; gpuIndex++)
        {
            const AGSDeviceInfo& device = gpuInfo.devices[gpuIndex];
            for (int i = 0; i < device.numDisplays; i++)
            {
                const AGSDisplayInfo& display = device.displays[i];
                if (display.displayFlags & AGS_DISPLAYFLAG_PRIMARY_DISPLAY && display.displayFlags & AGS_DISPLAYFLAG_HDR10)
                {

                    settings->chromaticityRedX = display.chromaticityRedX;               ///< Red display primary X coord
                    settings->chromaticityRedY = display.chromaticityRedY;               ///< Red display primary Y coord

                    settings->chromaticityGreenX = display.chromaticityGreenX;             ///< Green display primary X coord
                    settings->chromaticityGreenY = display.chromaticityGreenY;             ///< Green display primary Y coord

                    settings->chromaticityBlueX = display.chromaticityBlueX;              ///< Blue display primary X coord
                    settings->chromaticityBlueY = display.chromaticityBlueY;              ///< Blue display primary Y coord

                    settings->chromaticityWhitePointX = display.chromaticityWhitePointX;        ///< White point X coord
                    settings->chromaticityWhitePointY = display.chromaticityWhitePointY;        ///< White point Y coord

                    settings->minLuminance = display.minLuminance;                   ///< The minimum scene luminance in nits
                    settings->maxLuminance = display.maxLuminance;                   ///< The maximum scene luminance in nits

                    settings->maxContentLightLevel;           ///< The maximum content light level in nits (MaxCLL)
                    settings->maxFrameAverageLightLevel;
                    m_DeviceIndex = gpuIndex;
                    m_DisplayIndex = i;
                    m_bIsHDRAvailableOnPrimary = true;
                }
            }
        }
    }
}

bool cpMainComponents::AGSSetDisplay(AGSDisplaySettings *settings)
{
    if (AGS_SUCCESS == agsSetDisplayMode(m_agsContext, m_DeviceIndex, m_DisplayIndex, settings))
    {
        statusBar()->showMessage(tr("HDR enabled."));
        return true;
    }
    else
    {
        statusBar()->showMessage(tr("Set HDR fail."));
        return false;
    }
}
//---------------------------------------------------------------------------------


void cpMainComponents::handleHDRon()
{
    if (!m_bIsFullScreenModeOn)
    {
        this->showFullScreen();
        m_bIsFullScreenModeOn = true;
        onHDRButton->setText("Normal Screen");
    }
    else
    { 
        this->showNormal();
        if (m_bIsHDRAvailableOnPrimary)
            onHDRButton->setText("Full Screen with HDR");
        else
            onHDRButton->setText("Full Screen");
        m_bIsFullScreenModeOn = false;
    }

    // This part of the code could also be another button!
    // that is enabled if HDR is available on the primary display
    if (m_bIsHDRAvailableOnPrimary)
    {
        if (m_bIsFullScreenModeOn)
        {
            // HDR On
            m_settings.mode = AGSDisplaySettings::Mode_scRGB;
            if (AGSSetDisplay(&m_settings))
            {
            }
        }
        else
        {
            // HDR Off
            m_settings.mode = AGSDisplaySettings::Mode_SDR;
            if (AGSSetDisplay(&m_settings))
            {
            }
        }
    }

}
#endif

void cpMainComponents::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);
    settings.sync();
    SetProjectWindowTitle();
}

void cpMainComponents::updateRecentFileActions()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    QStringList files = settings.value("recentFileList").toStringList();
    QStringList UpdatedList;

    int numRecentFile = 0;
    int scan_numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
    
    UpdatedList.clear();
    for (int i = 0; i < scan_numRecentFiles; ++i)
    {

        m_projectsRecentFiles.push_back(files[i]);

        QFile file(files[i]);
        if (file.exists())
        {
            QString text = tr("&%1 %2").arg(numRecentFile + 1).arg(strippedName(files[i]));
            recentFileActs[numRecentFile]->setText(text);
            recentFileActs[numRecentFile]->setData(files[i]);
            recentFileActs[numRecentFile]->setStatusTip(files[i]);
            recentFileActs[numRecentFile]->setVisible(true);
            numRecentFile++;
            UpdatedList.append(files[i]);
        }
    }

    // Save back a cleaned up list of existing project files
    settings.setValue("recentFileList", UpdatedList);

    for (int j = numRecentFile; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
    
    m_numRecentFiles = numRecentFile;
    separatorAct->setVisible(m_numRecentFiles > 0);

}

QString cpMainComponents::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void cpMainComponents::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    if (fileMenu)
    {
        if (newProjectAct) fileMenu->addAction(newProjectAct);
        if (openAct) fileMenu->addAction(openAct);
        if (saveAct) fileMenu->addAction(saveAct);
        if (saveAsAct) fileMenu->addAction(saveAsAct);
        if (saveImageAct) fileMenu->addAction(saveImageAct);
        fileMenu->addSeparator();
        if (openImageFileAct) fileMenu->addAction(openImageFileAct);
        if (saveToBatchFileAct) fileMenu->addAction(saveToBatchFileAct);
        fileMenu->addSeparator();
        if (exitAct) fileMenu->addAction(exitAct);
        separatorAct = menuBar()->addSeparator();
        for (int i = 0; i < MaxRecentFiles; ++i)
            fileMenu->addAction(recentFileActs[i]);
        updateRecentFileActions();
        fileMenu->addSeparator();
        fileMenu->addAction(exitAct);
    }

    if (m_showAppSettingsDialog)
    {
        settingsMenu = menuBar()->addMenu(tr("&Settings"));
        if (settingsMenu)
        {
            if (settingsAct) settingsMenu->addAction(settingsAct);
        }
    }

    windowMenu = menuBar()->addMenu(tr("&Window"));
    if (windowMenu)
    {
        if (showWelcomePageAct)     windowMenu->addAction(showWelcomePageAct);
        if (showOutputAct)          windowMenu->addAction(showOutputAct);
        if (closeAllDocuments)      windowMenu->addAction(closeAllDocuments);
    }

    helpMenu = menuBar()->addMenu(tr("&Help"));
    if (helpMenu)
    {
        if (gettingStartedAct) helpMenu->addAction(gettingStartedAct);
        if (userGuideAct) helpMenu->addAction(userGuideAct);
        if (aboutAct) helpMenu->addAction(aboutAct);
    }
}

void cpMainComponents::menuItemClicked(QAction* triggeredAction)
{
    // use either the action itself... or an offset
    int value = triggeredAction->data().toInt();
    Q_UNUSED(value);
}

void cpMainComponents::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    if (fileToolBar)
    {
        if (newProjectAct) fileToolBar->addAction(newProjectAct);
        if (openAct) fileToolBar->addAction(openAct);
        if (saveAct) fileToolBar->addAction(saveAct);
        fileToolBar->addSeparator();
        if (openImageFileAct) fileToolBar->addAction(openImageFileAct);
        if (settingsAct) fileToolBar->addAction(settingsAct);
        fileToolBar->addSeparator();
        if (deleteImageAct) fileToolBar->addAction(deleteImageAct);
    }

    CompressionToolBar = addToolBar(tr("Compression"));
    if (CompressionToolBar)
    {
        if (compressAct) CompressionToolBar->addAction(compressAct);
        if (imagediffAct) CompressionToolBar->addAction(imagediffAct);
        if (MIPGenAct) CompressionToolBar->addAction(MIPGenAct);
#ifdef ENABLE_AGS_SUPPORT
        if (onHDRButton) CompressionToolBar->addWidget(onHDRButton);
#endif
    }

#ifdef USE_MAIN_IMAVEVIEW_TOOLBAR
    ImageViewToolBar = addToolBar(tr("Image View"));
    if (ImageViewToolBar)
    {
        if (imageview_zoomInAct) ImageViewToolBar->addAction(imageview_zoomInAct);
        if (imageview_zoomOutAct) ImageViewToolBar->addAction(imageview_zoomOutAct);
        if (imageview_RedAct) ImageViewToolBar->addAction(imageview_RedAct);
        if (imageview_GreenAct) ImageViewToolBar->addAction(imageview_GreenAct);
        if (imageview_BlueAct) ImageViewToolBar->addAction(imageview_BlueAct);
        if (imageview_AlphaAct) ImageViewToolBar->addAction(imageview_AlphaAct);
        if (imageview_FitScreenAct) ImageViewToolBar->addAction(imageview_FitScreenAct);
    }
    
    ImageViewToolBar->setEnabled(false);
#endif

}

void cpMainComponents::createStatusBar()
{
    statusBar()->setStyleSheet("QStatusBar{border-top: 1px outset grey;}");
    statusBar()->showMessage(tr("Ready"));
}

void cpMainComponents::showProgressBusy(QString Message)
{
    statusBar()->showMessage(Message);
    if (qApp)
        qApp->setOverrideCursor(Qt::BusyCursor);
    if (m_projectview)
        m_projectview->m_processBusy = true;
}

void cpMainComponents::hideProgressBusy(QString Message)
{
    statusBar()->showMessage(Message);
    qApp->restoreOverrideCursor();
    if (m_projectview)
        m_projectview->m_processBusy = false;
}

void cpMainComponents::readSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(800, 600)).toSize();
    resize(size);
    move(pos);
    if (m_showAppSettingsDialog)
    {
        m_setapplicationoptions->LoadSettings(m_sSettingsFile, QSettings::IniFormat);
    }
}

void cpMainComponents::writeSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    if (m_showAppSettingsDialog)
    {
        m_setapplicationoptions->SaveSettings(m_sSettingsFile, QSettings::IniFormat);
    }

}

acCustomDockWidget *cpMainComponents::FindImageView(QString &Title)
{
    QList<acCustomDockWidget *> dockWidgets = m_parent->findChildren<acCustomDockWidget *>();
    QListIterator<acCustomDockWidget *> iter(dockWidgets);
    acCustomDockWidget *dock;
    while (iter.hasNext())
    {
        dock = iter.next();
        QString m_fileName = dock->m_fileName;
        //qDebug() << "Match " << Title <<  "with TAB NAMES [" << m_fileName << "]";
        int res = Title.compare(m_fileName);
        if (res == 0) {
            //qDebug() << "--- FOUND ---";
            return dock;
        }
    }

    return NULL;
}

// Active when user selects any Docked Tabbed Item 
void cpMainComponents::onDockImageVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);

    QTabBar *tabBar = this->findChild<QTabBar *>();
    if (tabBar)
    {
        int currentIndex = tabBar->currentIndex();
        QString tabText = tabBar->tabText(currentIndex);
        int numTab = tabBar->count();

        for (int i = 0; i < numTab; i++)
        {
            if (tabBar->tabText(i).compare("") == 0)
            {
                tabBar->setStyleSheet("QTabBar::tab:disabled { width: 0; height: 0; margin: 0; padding: 0; border: none; }");
                tabBar->setTabEnabled(i, false);
            }
        }

        const QList<QDockWidget *> tabedWidgets = tabifiedDockWidgets(m_blankpage);
        QDockWidget *item;
        foreach(item, tabedWidgets) {

            // Get our custom Dock Widget 
            if (item->titleBarWidget())
            {
                acCustomDockWidget *imageItem = reinterpret_cast<acCustomDockWidget *> (item);
                if (imageItem && !m_viewDiff)
                {
                    if (imageItem->m_tabName.compare("") == 0)
                    {
                        imageItem->lower();
                    }

                    if (imageItem->m_tabName.compare(tabText) == 0)
                    {
                        emit SetCurrentItem(imageItem->m_fileName);
                    }
                }
            }
        }
    }
}

void cpMainComponents::AddImageCompSettings(QTreeWidgetItem *item, C_Destination_Options &data)
{
    if (!item) return;

    if (data.m_editing)
    {
        QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
        C_Destination_Options *m_data = v.value<C_Destination_Options *>();
        *m_data << data;
        item->setText(0, m_data->m_compname);

        QFileInfo fileinfo(m_data->m_destFileNamePath);
        QFile file(m_data->m_destFileNamePath);
        m_data->m_FileSize = file.size();
        if (m_data->m_FileSize > 1024000)
            m_data->m_FileSizeStr = QString().number((double)m_data->m_FileSize / 1024000, 'f', 2) + " MB";
        else
            if (m_data->m_FileSize > 1024)
                m_data->m_FileSizeStr = QString().number((double)m_data->m_FileSize / 1024, 'f', 1) + " KB";
            else
                m_data->m_FileSizeStr = QString().number(m_data->m_FileSize) + " Bytes";
        if (file.exists() && (fileinfo.suffix().length() > 0))
            item->setIcon(0, QIcon(":/CompressonatorGUI/Images/smallGrayStone.png"));
        else
            item->setIcon(0, QIcon(":/CompressonatorGUI/Images/smallWhiteBlank.png"));

        // refresh the current Image property view (It may or maynot be pointing to The compression data 
        // That was edited.
        m_imagePropertyView->refreshView();

        // Refresh the image
        m_projectview->m_clicked_onIcon = true;
        m_projectview->onTree_ItemClicked(item, 0);

    }
    else
    {
        C_Destination_Options *m_data = new C_Destination_Options;
        // copythe new data from comsettings dialog data
        *m_data << data;

        // Check who called the [+] add setting 
        QString itemName;
        itemName = item->text(0);
        QVariant v = item->data(TREE_LevelType, Qt::UserRole);
        int levelType = v.toInt();

        switch (levelType)
        {
            case TREETYPE_Add_destination_setting:
                    m_projectview->Tree_AddCompressFile(item, m_data->m_compname, true, true, TREETYPE_COMPRESSION_DATA, m_data);
                    break;
            case TREETYPE_Add_glTF_destination_settings:
                    QString DestfilePathName = m_data->m_destFileNamePath;
                    if (QFile::exists(m_data->m_destFileNamePath)) 
                    {
                        QFile::remove(m_data->m_destFileNamePath);
                    }
                    bool isCopy = QFile::copy(m_data->m_sourceFileNamePath, m_data->m_destFileNamePath);
                    if (!isCopy) {
                        QString error = "Error: Create " + (m_data->m_destFileNamePath) + " failed.\n";
                        PrintInfo(error.toStdString().c_str());
                        return;
                    }
                    QTreeWidgetItem *ParentItem = item->parent();
                    if (ParentItem)
                    {
                        QString itemName = ParentItem->text(0);
                        m_projectview->Tree_Add3DSubModelFile(ParentItem, DestfilePathName);
                    }
                    break;
        }
    }
}

void cpMainComponents::AddImageView(QString &fileName, QTreeWidgetItem * item)
{
    bool isDel = true;
    bool doRefreshCompressView = false;

    try
    {
        if (isCompressInProgress) return;

        if (!item) return;

        emit OnImageLoadStart();

        if (deleteImageAct)
        {
            isDel = deleteImageAct->isEnabled();
            deleteImageAct->setEnabled(false);
        }

        // Determin File Type
        QVariant v = item->data(TREE_LevelType, Qt::UserRole);
        int levelType = v.toInt();

        acCustomDockWidget      *dock = NULL;
        C_Destination_Options   *m_compressdata;

        if (levelType == TREETYPE_COMPRESSION_DATA)
        {
            QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
            m_compressdata = v.value<C_Destination_Options *>();
            if (m_compressdata->m_data_has_been_changed)
            {
                // Find the old image and remove it
                OnDeleteImageView(fileName);
                m_compressdata->m_data_has_been_changed = false;
                doRefreshCompressView = true;
            }
        }

        if (((  g_Application_Options.m_useNewImageViews
                || doRefreshCompressView
                || g_Application_Options.m_refreshCurrentView 
                || m_ForceImageRefresh))
                && (levelType != TREETYPE_3DMODEL_DATA)
                && (levelType != TREETYPE_3DSUBMODEL_DATA)
            )
        {
            // Find the old image and remove it
            OnDeleteImageView(fileName);

            if (doRefreshCompressView)
                m_compressdata->m_data_has_been_changed = false;

            m_ForceImageRefresh = false;
        }
        else
        {
            // Make sure we are not not already viewing this image file
            dock = FindImageView(fileName);
        }

        if (dock) {
            // We found the image in our list of existing views
            m_activeImageTab = dock;
        }

        showProgressBusy("Loading Image... Please wait");

        if (!g_Application_Options.m_useNewImageViews)
        {
            if (dock) {
                // check if the view is uptodate 
                // or needs to be refreshed
                m_activeImageTab = dock;

                if (levelType == TREETYPE_COMPRESSION_DATA)
                {
                    if (doRefreshCompressView)
                    {
                        // Flag project we have new project settings to save on exit!
                        if (m_projectview)
                            m_projectview->m_saveProjectChanges = true;

                        // Remove the old tab 
                        if (dock)
                        {
                            delete dock;
                            dock = NULL;
                        }
                    }
                }
            }
        }

        if (dock == NULL)
        {
            QString ImageType;
            Setting *setting = new Setting();
            setting->onBrightness = false;

            if (levelType == TREETYPE_COMPRESSION_DATA)
            {
                // Get ImageFile Data
                QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
                C_Destination_Options *m_filedata = v.value<C_Destination_Options *>();

                if (m_filedata)
                {
                    QFile file(fileName);
                    m_filedata->m_FileSize = file.size();
                    if (m_filedata->m_FileSize > 1024000)
                        m_filedata->m_FileSizeStr = QString().number((double)m_filedata->m_FileSize / 1024000, 'f', 2) + " MB";
                    else
                        if (m_filedata->m_FileSize > 1024)
                            m_filedata->m_FileSizeStr = QString().number((double)m_filedata->m_FileSize / 1024, 'f', 1) + " KB";
                        else
                            m_filedata->m_FileSizeStr = QString().number(m_filedata->m_FileSize) + " Bytes";

                    // Try to get the root node 
                    // for this compressed image view
                    CMipImages *OrigImages = NULL;
                    QTreeWidgetItem *parent = item->parent();
                    if (parent)
                    {
                        QVariant v = parent->data(TREE_SourceInfo, Qt::UserRole);
                        C_Source_Info *imagedata = v.value<C_Source_Info *>();
                        if (imagedata)
                            OrigImages = imagedata->m_MipImages;
                    }

                    // Create a new view image
                    ImageType = " Image file";
                    m_imageview = new cpImageView(fileName, ImageType, m_parent, m_filedata->m_MipImages, setting, OrigImages);
                }
            }
            else
            if ((levelType == TREETYPE_IMAGEFILE_DATA) || (levelType == TREETYPE_VIEW_ONLY_NODE))
            {
                // Get ImageFile Data
                QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
                C_Source_Info *m_filedata = v.value<C_Source_Info *>();

                if (m_filedata)
                {
                    // Create a new view image
                    ImageType = " Original Image file";
                    setting->reloadImage = g_Application_Options.m_useNewImageViews;
                    if (m_filedata->m_MipImages->Image_list.count() > 1)
                        setting->generateMips = true;

                    m_imageview = new cpImageView(fileName, ImageType, m_parent, m_filedata->m_MipImages, setting, NULL);

                    setting->generateMips = false;
                }
            }
            else
            if ((levelType == TREETYPE_3DMODEL_DATA)|| (levelType == TREETYPE_3DSUBMODEL_DATA))
            {
                m_imageview = NULL;
                // Get ImageFile Data
                QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
                C_3D_Source_Info *m_filedata = v.value<C_3D_Source_Info *>();

                if (m_filedata)
                {
                    // Create a new view image
                    ImageType = " 3D Model DX12 Render";
                    setting->reloadImage = g_Application_Options.m_useNewImageViews;
                    setting->generateMips = false;
                    m_3Dmodelview = new cp3DModelView(fileName, ImageType, m_parent);
                    setting->generateMips = false;
                }
            }

            if (m_imageview)
            {
                m_imageview->showToobar(true);
                m_imageview->showToobarButton(true);
                m_imageview->setAllowedAreas(Qt::RightDockWidgetArea);
                m_parent->addDockWidget(Qt::RightDockWidgetArea, m_imageview);
                m_parent->tabifyDockWidget(m_blankpage, m_imageview);
                m_viewDiff = false;
                connect(m_imageview, SIGNAL(visibilityChanged(bool)), this, SLOT(onDockImageVisibilityChanged(bool)));
                connect(m_imageview, SIGNAL(UpdateData(QObject *)), m_imagePropertyView, SLOT(OnUpdateData(QObject *)));
                m_activeImageTab = m_imageview;
            }

            if (m_3Dmodelview)
            {
                m_3Dmodelview->setAllowedAreas(Qt::RightDockWidgetArea);
                m_parent->addDockWidget(Qt::RightDockWidgetArea, m_3Dmodelview);
                m_parent->tabifyDockWidget(m_blankpage, m_3Dmodelview);
                m_viewDiff = false;
                connect(m_3Dmodelview, SIGNAL(visibilityChanged(bool)), this, SLOT(onDockImageVisibilityChanged(bool)));
                connect(m_3Dmodelview, SIGNAL(UpdateData(QObject *)), m_imagePropertyView, SLOT(OnUpdateData(QObject *)));
                m_activeImageTab = m_3Dmodelview;
            }
        }

        // =================================
        // Place the TAB on view if hidden!
        // =================================
        if (m_activeImageTab)
        {
            // check its visability: User is requesting view
            if (!m_activeImageTab->isVisible())
            {
                m_activeImageTab->setVisible(true);
            }

            m_activeImageTab->raise();
            //QTimer::singleShot(10, this, SLOT(SetRaised()));
        }
    }
    catch (...)
    {
        if (m_CompressStatusDialog)
        {
            m_CompressStatusDialog->showOutput();
            m_CompressStatusDialog->raise();
            //QTimer::singleShot(10, this, SLOT(SetRaised()));
        }
          // do some message 
    }

    emit OnImageLoadDone();
  
    if (deleteImageAct)
        deleteImageAct->setEnabled(isDel);
    if (saveImageAct)
        saveImageAct->setEnabled(true);

    hideProgressBusy("Ready");
}

void cpMainComponents::AddImageDiff(C_Destination_Options *destination, QString &fileName1, QString &fileName2)
{
    try
    {
        if (isCompressInProgress) return;

        bool isComp = true, isDel = true;

        emit OnImageLoadStart();
        //if (compressAct)
        //{
        //    isComp = compressAct->isEnabled();
        //    compressAct->setEnabled(false);
        //}

        if (imagediffAct)
        {
            imagediffAct->setEnabled(false);
        }

        if (deleteImageAct)
        {
            isDel = deleteImageAct->isEnabled();
            deleteImageAct->setEnabled(false);
        }

        if (m_CompressStatusDialog)
        {
            m_CompressStatusDialog->onClearText();
            m_CompressStatusDialog->showOutput();
        }

        showProgressBusy("Loading Image Differance...Please wait");

        QString originalFileName = ""; 
        QString destFile = "";
        QString title = "";

        if (destination == NULL)
        {
            QFileInfo fileinfo1(fileName1);
            QFile file1(fileName1);
            if (file1.exists() && (fileinfo1.suffix().length() > 0))
            {
                originalFileName = fileName1;
            }
            else
            {
                PrintInfo("Image Diff Error: Image File #1 cannot be found\n");
                onShowOutput();
            }

            QFileInfo fileinfo2(fileName2);
            QFile file2(fileName2);
            if (file2.exists() && (fileinfo2.suffix().length() > 0))
            {
                destFile = fileName2;
            }
            else
            {
                PrintInfo("Image Diff Error: Image File #2 cannot be found\n");
                onShowOutput();
            }

            title = DIFFERENCE_IMAGE_TXT + originalFileName + " VS " + destFile;
            OnDeleteImageDiffView(destFile);

            // Create a new view image
            m_imageCompare = new CImageCompare(title, originalFileName, destFile, false, this);
        }
        else
        {
            originalFileName = destination->m_sourceFileNamePath;
            // Find the old image diff and remove it
            // User may have selected a newer Decompression Option
            // for the image diff view
            title = DIFFERENCE_IMAGE_TXT + destination->m_destFileNamePath;
            OnDeleteImageDiffView(destination->m_destFileNamePath);

            
            if (QDir(destination->m_decompressedFileNamePath).exists() && (QFile(destination->m_decompressedFileNamePath).size()) > 0)
            {
                destFile = destination->m_decompressedFileNamePath;
            }
            else
            {
                destFile = destination->m_destFileNamePath;
            }

            // Create a new view image
            m_imageCompare = new CImageCompare(title, originalFileName, destFile, true, this);
        }
        
       
        CMipImages  *m_diffMips = m_imageCompare->getMdiffMips();
        if (m_diffMips == NULL)
        {
            delete m_imageCompare;
            PrintInfo("Image Diff Error: Diff Image cannot be found\n");
        }
        else
        {
            m_imageCompare->setAllowedAreas(Qt::RightDockWidgetArea);
            m_parent->addDockWidget(Qt::RightDockWidgetArea, m_imageCompare);
            m_parent->tabifyDockWidget(m_blankpage, m_imageCompare);
            if (m_imageCompare->custTitleBar)
            {
                m_imageCompare->custTitleBar->m_close = true;
                connect(m_imageCompare->custTitleBar, SIGNAL(OnAboutToClose(QString &)), this, SLOT(onAboutToClose(QString &)));

            }
            m_viewDiff = true;
            connect(m_imageCompare, SIGNAL(visibilityChanged(bool)), this, SLOT(onDockImageVisibilityChanged(bool)));
            connect(m_imageCompare, SIGNAL(UpdateData(QObject *)), m_imagePropertyView, SLOT(OnUpdateData(QObject *)));
            m_activeImageTab = m_imageCompare;

            // check its visability: User is requesting view
            if (!m_activeImageTab->isVisible())
            {
                m_activeImageTab->setVisible(true);
            }

            m_activeImageTab->raise();
            QTimer::singleShot(30, this, SLOT(SetRaised()));
        }


        hideProgressBusy("Ready");

        emit OnImageLoadDone();

        if (imagediffAct)
            imagediffAct->setEnabled(isComp);
        if (deleteImageAct)
            deleteImageAct->setEnabled(isDel);
    }
    catch (...)
    {
        // Do some message
    }

}


void cpMainComponents::Add3DModelDiff(C_3D_Source_Info *destination, QString &fileName1, QString &fileName2)
{
    try
    {
        if (isCompressInProgress) return;

        bool isComp = true, isDel = true;

        emit OnImageLoadStart();

        if (imagediffAct)
        {
            imagediffAct->setEnabled(false);
        }

        if (deleteImageAct)
        {
            isDel = deleteImageAct->isEnabled();
            deleteImageAct->setEnabled(false);
        }

        if (m_CompressStatusDialog)
        {
            m_CompressStatusDialog->onClearText();
            m_CompressStatusDialog->showOutput();
        }

        showProgressBusy("Loading Image Differance...Please wait");

        QString originalFileName = "";
        QString destFile = "";
        QString title = "";

        QFileInfo fileinfo1(fileName1);
        QFile file1(fileName1);
        if (file1.exists() && (fileinfo1.suffix().length() > 0))
        {
            originalFileName = fileName1;
        }
        else
        {
            PrintInfo("Image Diff Error: Image File #1 cannot be found\n");
            onShowOutput();
        }

        QFileInfo fileinfo2(fileName2);
        QFile file2(fileName2);
        if (file2.exists() && (fileinfo2.suffix().length() > 0))
        {
            destFile = fileName2;
        }
        else
        {
            PrintInfo("Image Diff Error: Image File #2 cannot be found\n");
            onShowOutput();
        }

        title = DIFFERENCE_IMAGE_TXT + originalFileName + " VS " + destFile;
        OnDeleteImageDiffView(destFile);

        // Create a new view image
        m_3dModelCompare = new C3DModelCompare(title, fileName1, fileName2, true, this);

       // CMipImages  *m_diffMips = m_3dModelCompare->getMdiffMips();
       // if (m_diffMips == NULL)
       // {
       //     delete m_3dModelCompare;
       //     PrintInfo("Image Diff Error: Diff Image cannot be found\n");
       // }
       // else
        {
            m_3dModelCompare->setAllowedAreas(Qt::RightDockWidgetArea);
            m_parent->addDockWidget(Qt::RightDockWidgetArea, m_3dModelCompare);
            m_parent->tabifyDockWidget(m_blankpage, m_3dModelCompare);
            if (m_3dModelCompare->custTitleBar)
            {
                m_3dModelCompare->custTitleBar->m_close = true;
                connect(m_3dModelCompare->custTitleBar, SIGNAL(OnAboutToClose(QString &)), this, SLOT(onAboutToClose(QString &)));

            }
            m_viewDiff = true;
            connect(m_3dModelCompare, SIGNAL(visibilityChanged(bool)), this, SLOT(onDockImageVisibilityChanged(bool)));
            connect(m_3dModelCompare, SIGNAL(UpdateData(QObject *)), m_imagePropertyView, SLOT(OnUpdateData(QObject *)));
            m_activeImageTab = m_3dModelCompare;

            // check its visability: User is requesting view
            if (!m_activeImageTab->isVisible())
            {
                m_activeImageTab->setVisible(true);
            }

            m_activeImageTab->raise();
            QTimer::singleShot(30, this, SLOT(SetRaised()));
        }


        hideProgressBusy("Ready");

        emit OnImageLoadDone();

        if (imagediffAct)
            imagediffAct->setEnabled(isComp);
        if (deleteImageAct)
            deleteImageAct->setEnabled(isDel);
    }
    catch (...)
    {
        // Do some message
    }

}


void cpMainComponents::OnDeleteImageView(QString &fileName)
{
    showProgressBusy("Removing Image view ... Please wait");
    // Make sure we are not not already viewing this image file
    acCustomDockWidget *dock = (acCustomDockWidget *)FindImageView(fileName);

    if (dock)
    {
        delete dock;
        dock = NULL;
    }

    // Also Remove any Image Diff Tabs
    OnDeleteImageDiffView(fileName);

    hideProgressBusy("Ready");
}

void cpMainComponents::OnDeleteImageDiffView(QString &fileName)
{
    showProgressBusy("Removing Image Differance view ... Please wait");

    QString Title = DIFFERENCE_IMAGE_TXT + fileName;

    // Make sure we are not not already viewing this image file
    acCustomDockWidget *dock = (acCustomDockWidget *) FindImageView(Title);

    if (dock)
    {
        delete dock;
        dock = NULL;
    }

    hideProgressBusy("Ready");
}

void cpMainComponents::SetRaised()
{
    if (m_activeImageTab)
        m_activeImageTab->raise();
}

cpMainComponents::~cpMainComponents()
{
    g_bAbortCompression = true;
    CMP_ShutdownDecompessLibrary();
}

void cpMainComponents::OnAddCompressSettings(QTreeWidgetItem *item)
{
    if (!item) return;

    QVariant v = item->data(TREE_LevelType, Qt::UserRole);
    int levelType = v.toInt();

    //int setting = 1;
    QString CompProjectName = "New";

    // Obtain the Parent and its data
    QTreeWidgetItem *parent = item->parent();

    // if no parent verify item itself as parent
    if (!parent)
        parent = item;

    if (parent)
    {
        // Verify its root
        QVariant v = parent->data(TREE_LevelType, Qt::UserRole);
        int itemlevelType = v.toInt();
        
        if (itemlevelType == TREETYPE_IMAGEFILE_DATA)
        {
            QVariant v = parent->data(TREE_SourceInfo, Qt::UserRole);
            C_Source_Info *m_sourcefile = v.value<C_Source_Info *>();

            QFileInfo fileinfo(m_sourcefile->m_Name);
            CompProjectName = fileinfo.baseName();

            m_setcompressoptions->m_data.init();
            m_setcompressoptions->m_data.m_sourceFileNamePath       = m_sourcefile->m_Full_Path;
            m_setcompressoptions->m_data.m_SourceImageSize          = m_sourcefile->m_ImageSize;
            m_setcompressoptions->m_data.m_SourceIscompressedFormat = CompressedFormat(m_sourcefile->m_Format);
            m_setcompressoptions->m_data.m_SourceIsFloatFormat      = FloatFormat(m_sourcefile->m_Format);

            // Used to append to name - for unique name
            // There is still chances of duplucate names, but it will not effect
            // compression unless target file is also of the same name as any other child 
            // compression settings
            int parentcount = parent->childCount();

            if (m_sourcefile->m_extnum <= parentcount)
                m_sourcefile->m_extnum = parentcount;

            // Extension Counter Number incriment
            m_setcompressoptions->m_extnum = m_sourcefile->m_extnum++;

            // Set image target size
            m_setcompressoptions->m_data.m_Width  = m_sourcefile->m_Width;
            m_setcompressoptions->m_data.m_Height = m_sourcefile->m_Height;

            // Set Compression Widgets to enable
            m_setcompressoptions->m_showDestinationEXTSetting = true;
            m_setcompressoptions->m_showTheControllerSetting  = true;
            m_setcompressoptions->m_showTheInfoTextSetting    = true;

            // List of source files - for am image file there is only one source file
            // clean up combo list
            m_setcompressoptions->m_CBSourceFile->clear();

            QFileInfo fi(m_setcompressoptions->m_data.m_sourceFileNamePath);
            QString name = fi.fileName();
            m_setcompressoptions->m_CBSourceFile->addItem(name);
        }
        else
        if (itemlevelType == TREETYPE_3DMODEL_DATA)
        {
            QVariant v = parent->data(TREE_SourceInfo, Qt::UserRole);
            C_3D_Source_Info *m_sourcefile = v.value<C_3D_Source_Info *>();
            
            // Source File Name
            QFileInfo fileinfo(m_sourcefile->m_Name);
            CompProjectName = fileinfo.baseName();

            m_setcompressoptions->m_data.init();
            m_setcompressoptions->m_data.m_sourceFileNamePath = m_sourcefile->m_Full_Path;

            m_sourcefile->m_extnum++;

            // Extension Counter Number incriment
            m_setcompressoptions->m_extnum = m_sourcefile->m_extnum++;

            // List of source files
            if (levelType == TREETYPE_Add_glTF_destination_settings)
            {
                // Set Compression Widgets to enable
                m_setcompressoptions->m_showDestinationEXTSetting   = false;
                m_setcompressoptions->m_showTheControllerSetting    = false;
                m_setcompressoptions->m_showTheInfoTextSetting      = false;
                m_setcompressoptions->m_CBSourceFile->clear();
                QFileInfo fi(m_setcompressoptions->m_data.m_sourceFileNamePath);
                QString name = fi.fileName();
                m_setcompressoptions->m_CBSourceFile->addItem(name);
                m_setcompressoptions->m_data.m_modelSource = m_sourcefile->m_modelSource;
            }
            else
            {
                m_setcompressoptions->m_CBSourceFile->clear();
            }
           
        }
        else
        if (itemlevelType == TREETYPE_3DSUBMODEL_DATA)
        {
            if (levelType != TREETYPE_Add_destination_setting) return; // noting to do with compression settings!

            // copy 3d sub src data from 3d src models
            QVariant v = parent->data(TREE_SourceInfo, Qt::UserRole);
            C_3D_Source_Info *m_sourcefile = v.value<C_3D_Source_Info *>();
            if (!m_sourcefile) return;

            // all 3d src del flag set to true -> all gltf src setting added
            if (!(m_sourcefile->m_srcDelFlags.contains(false))) {
                PrintInfo("Error: All setting for the textures in the gltf file have been added. \n Note: To add/ modify setting, please add new gltf setting or modify the current setting through properties window.");
                onShowOutput();
                return;
            }

            m_setcompressoptions->m_data.m_sourceFiles = m_sourcefile->m_sourceFiles;
            m_setcompressoptions->m_data.m_srcDelFlags = m_sourcefile->m_srcDelFlags;

            // Set Compression Widgets to enable
            m_setcompressoptions->m_showDestinationEXTSetting = true;
            m_setcompressoptions->m_showTheControllerSetting = true;
            m_setcompressoptions->m_showTheInfoTextSetting = true;
          
            m_setcompressoptions->m_CBSourceFile->clear();
            for (int i = 0; i < m_setcompressoptions->m_data.m_sourceFiles.size(); ++i)
            {
                QFileInfo fi(m_setcompressoptions->m_data.m_sourceFiles.at(i));
                QString name = fi.fileName();
                // check for delete flags
                if(m_setcompressoptions->m_data.m_srcDelFlags.at(i) == false)
                    m_setcompressoptions->m_CBSourceFile->addItem(name);
            }

            // Default to first entry
            m_setcompressoptions->m_data.m_sourceFileNamePath = m_sourcefile->m_sourceFiles.at(0);
            m_setcompressoptions->m_data.m_modelDest = m_sourcefile->m_Full_Path;
            m_setcompressoptions->m_data.m_modelSource = m_sourcefile->m_modelSource;

            QFileInfo fileinfo(m_setcompressoptions->m_data.m_sourceFileNamePath);
            CompProjectName = fileinfo.baseName();

            int parentcount = parent->childCount();
            if (m_sourcefile->m_extnum <= parentcount)
                m_sourcefile->m_extnum = parentcount;

            // Extension Counter Number incriment
            m_setcompressoptions->m_extnum = m_sourcefile->m_extnum++;

        }

        m_setcompressoptions->m_data.m_SourceType = itemlevelType;
    }

#ifdef USE_OLD1
    else 
    if (!parent)
    {
        // Verify item itself
        QVariant v = item->data(TREE_LevelType, Qt::UserRole);
        int itemlevelType = v.toInt();
        m_setcompressoptions->m_data.m_SourceType = itemlevelType;
        if (itemlevelType == TREETYPE_IMAGEFILE_DATA)
        {
            parent = item;
            QVariant v = parent->data(TREE_SourceInfo, Qt::UserRole);
            C_Source_Info *m_sourcefile = v.value<C_Source_Info *>();
            QFileInfo fileinfo(m_sourcefile->m_Name);
            CompProjectName = fileinfo.baseName();
            m_setcompressoptions->m_data.m_sourceFileNamePath = m_sourcefile->m_Full_Path;
            m_setcompressoptions->m_data.m_SourceImageSize = m_sourcefile->m_ImageSize;
            m_setcompressoptions->m_data.m_SourceIscompressedFormat = CompressedFormat(m_sourcefile->m_Format);
            m_setcompressoptions->m_data.m_SourceIsFloatFormat = FloatFormat(m_sourcefile->m_Format);

            // Used to append to name - for unique name
            // There is still chances of duplucate names, but it will not effect
            // compression unless target file is also of the same name as any other child 
            // compression settings
            int parentcount = parent->childCount();

            if (m_sourcefile->m_extnum <= parentcount)
                m_sourcefile->m_extnum = parentcount;

            m_setcompressoptions->m_extnum = m_sourcefile->m_extnum++;
            m_setcompressoptions->m_data.m_Width  = m_sourcefile->m_Width;
            m_setcompressoptions->m_data.m_Height = m_sourcefile->m_Height;

            m_setcompressoptions->m_showDestinationEXTSetting = true;
            m_setcompressoptions->m_showTheControllerSetting  = true;
            m_setcompressoptions->m_showTheInfoTextSetting    = true;

            // List of source files
            m_setcompressoptions->m_sourceFiles.clear();
        }
        else
            if (itemlevelType == TREETYPE_3DSUBMODEL_DATA)
            {
                parent = item;
                QVariant v = parent->data(TREE_SourceInfo, Qt::UserRole);
                C_3D_Source_Info *m_sourcefile = v.value<C_3D_Source_Info *>();
                QFileInfo fileinfo(m_sourcefile->m_Name);
                CompProjectName = fileinfo.baseName();
                m_setcompressoptions->m_data.m_sourceFileNamePath = m_sourcefile->m_Full_Path;
                int parentcount = parent->childCount();
                if (m_sourcefile->m_extnum <= parentcount)
                    m_sourcefile->m_extnum = parentcount;

                // Extension Counter Number incriment
                m_setcompressoptions->m_extnum = m_sourcefile->m_extnum++;

                // Set Compression Widgets to enable
                m_setcompressoptions->m_showDestinationEXTSetting = false;
                m_setcompressoptions->m_showTheControllerSetting = false;
                m_setcompressoptions->m_showTheInfoTextSetting = false;

                // List of source files
                m_setcompressoptions->m_sourceFiles = m_sourcefile->m_sourceFiles;
            }
        else
        if (itemlevelType == TREETYPE_3DMODEL_DATA)
        {
            parent = item;
            QVariant v = parent->data(TREE_SourceInfo, Qt::UserRole);
            C_3D_Source_Info *m_sourcefile = v.value<C_3D_Source_Info *>();
            QFileInfo fileinfo(m_sourcefile->m_Name);
            CompProjectName = fileinfo.baseName();
            m_setcompressoptions->m_data.m_sourceFileNamePath = m_sourcefile->m_Full_Path;
            int parentcount = parent->childCount();
            if (m_sourcefile->m_extnum <= parentcount)
                m_sourcefile->m_extnum = parentcount;

            // Extension Counter Number incriment
            m_setcompressoptions->m_extnum = m_sourcefile->m_extnum++;

            // Set Compression Widgets to enable
            m_setcompressoptions->m_showDestinationEXTSetting   = true;
            m_setcompressoptions->m_showTheControllerSetting    = true;
            m_setcompressoptions->m_showTheInfoTextSetting      = true;

            // List of source files
            m_setcompressoptions->m_sourceFiles = m_sourcefile->m_sourceFiles;
            // Default to first entry
            if (m_sourcefile->m_sourceFiles.size() > 0)
                m_setcompressoptions->m_data.m_sourceFileNamePath = m_sourcefile->m_sourceFiles.at(0);
            else
                return;
        }
    }
#endif

    m_setcompressoptions->m_data.m_compname = CompProjectName;

    m_setcompressoptions->m_data.m_editing = false;
    m_setcompressoptions->m_item = item;                    

    emit m_setcompressoptions->m_data.compressionChanged((QVariant &)m_setcompressoptions->m_data.m_Compression);

    if (m_setcompressoptions->updateDisplayContent())
    {
        if (!m_setcompressoptions->isVisible())
        {
            QPoint pos = QCursor::pos();
            m_setcompressoptions->move(pos);
            m_setcompressoptions->show();
        }
    }

}

void cpMainComponents::onAddedCompressSettingNode()
{
    //if (compressAct)
    //    compressAct->setEnabled(true);
    m_setcompressoptions->m_destFilePath = m_setcompressoptions->m_DestinationFolder->text();
}

void cpMainComponents::onEditCompressSettings(QTreeWidgetItem *item)
{
    QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
    C_Destination_Options *m_data = v.value<C_Destination_Options *>();
    if (m_data)
    {
        m_setcompressoptions->m_item = item;
        m_setcompressoptions->m_data << (const C_Destination_Options &)*m_data;

        if (m_setcompressoptions->updateDisplayContent())
        {
            if (!m_setcompressoptions->isVisible())
            {
                QPoint pos = QCursor::pos();
                m_setcompressoptions->move(pos);
                m_setcompressoptions->show();
            }

        }
    }
}

//==========================================
// Static Members used to redirect 
// Messages to GUI
//==========================================
bool isCompressMSG = false;
QString comError = "Failed to initialize COM";

// ----------------------------------
// Messages from command line prints
// ----------------------------------

void cpMainComponents::PrintStatus(char *buff)
{
    //qDebug() << buff;
    isCompressMSG = true;
    QString msg = buff;
    if (msg.contains(comError) )
                            return;

    emit static_msghandler.signalMessage(buff);
}

// ----------------------------------
// Messages from qDebug()
// ----------------------------------

void cpMainComponents::msgHandler(QtMsgType type, const char* msg)
{
    Q_UNUSED(type);
    Q_UNUSED(msg);
    //emit static_msghandler.signalMessage(msg);
}

void cpMainComponents::browserMsg(const char *msg)
{
    //statusBar()->showMessage(msg);
    if (m_CompressStatusDialog && isCompressMSG)
    {
        QString qmsg = msg;
        qmsg.remove(QRegExp("[\\n\\r]"));
        m_CompressStatusDialog->appendText(qmsg);
        isCompressMSG = false;
    }
}

//================================================

void cpMainComponents::removeItemTabs(QString *FilePathName)
{
    QTreeWidgetItem *item = m_projectview->GetCurrentItem(TREETYPE_COMPRESSION_DATA);
    if (item)
    {
        // qDebug() << "Delete this Tab: " << *FilePathName;
        // view image
        QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
        C_Destination_Options *m_data = v.value<C_Destination_Options *>();
        if (m_data)
        {
            if ((m_data->m_destFileNamePath.compare(*FilePathName) == 0) ||
                (m_data->m_destFileNamePath.compare(DIFFERENCE_IMAGE_TXT + *FilePathName) == 0))
            {
                OnDeleteImageView(*FilePathName);
                OnDeleteImageDiffView(*FilePathName);
                m_activeImageTab = NULL;
                if (m_projectview)
                {
                    m_projectview->Tree_updateCompressIcon(item, *FilePathName, false);
                    m_projectview->m_saveProjectChanges = true;
                }

            }
        }
    }
}

void cpMainComponents::onPropertyViewSaveSetting(QString *FilePathName)
{
    if (m_projectview)
    {
        QFile::remove(*FilePathName);
        removeItemTabs(FilePathName);
        m_projectview->Tree_SetCurrentItem(*FilePathName);
    }
}

void cpMainComponents::onPropertyViewCompressImage(QString *FilePathName)
{
    if (m_projectview)
    {
        removeItemTabs(FilePathName);
        m_projectview->Tree_clearAllItemsSetected();
        QTreeWidgetItem *item = m_projectview->Tree_SetCurrentItem(*FilePathName);
        if (item != NULL)
        {
            QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
            C_Destination_Options *m_data = v.value<C_Destination_Options *>();
            if (m_data)
            {
                m_data->m_isselected = true;
                m_projectview->OnStartCompression();
            }
        }
    }
}

void cpMainComponents::onCompressionStart()
{
    // Disable relavent tool bar options
    if (compressAct)
        compressAct->setEnabled(false);
    if (imagediffAct)
        imagediffAct->setEnabled(false);
    if (deleteImageAct)
        deleteImageAct->setEnabled(false);
    if (MIPGenAct)
        MIPGenAct->setEnabled(false);

    // Free up as much memory as we can 
    // prior to processing
    if (g_Application_Options.m_closeAllDocuments)
        onCloseAllDocuments();

    isCompressInProgress = true;
}

void cpMainComponents::onCompressionDone()
{
    isCompressInProgress = false;

    // Called when compression from Project view is completed.
    QTreeWidgetItem *item = m_projectview->GetCurrentItem(TREETYPE_COMPRESSION_DATA);
    if (item)
    {

        QTreeWidgetItem *parent = item->parent();
        if (parent)
        {
            // Verify its root
            QVariant v = parent->data(TREE_LevelType, Qt::UserRole);
            int ParentlevelType = v.toInt();
            if (ParentlevelType == TREETYPE_3DSUBMODEL_DATA)
            {
                QVariant data = item->data(TREE_SourceInfo, Qt::UserRole);
                C_Destination_Options *m_data = data.value<C_Destination_Options *>();
                if (m_data)
                {
                    std::ifstream fstreamsrc(m_data->m_modelSource.toStdString());
                    if (!fstreamsrc)
                    {
                        PrintInfo("Error reading gltf source file.");
                        return;
                    }

                    std::ifstream fstreamdest(m_data->m_modelDest.toStdString());
                    if (!fstreamdest)
                    {
                        PrintInfo("Error reading gltf compressed version file.");
                        return;
                    }

                    // Load the glTF json text file
                    nlohmann::json gltfsrc;
                    fstreamsrc >> gltfsrc;
                    fstreamsrc.close();

                    nlohmann::json gltfdest;
                    fstreamdest >> gltfdest;
                    fstreamdest.close();

                    //image section of gltf file
                    auto srcimages = gltfsrc["images"];

                    for (unsigned int i = 0; i < srcimages.size(); i++)
                    {
                        std::string srcname = srcimages[i]["uri"].get<std::string>();
                        QFileInfo fileInfo(m_data->m_sourceFileNamePath);
                        QString filename(fileInfo.fileName());
                        std::string srcfilename = filename.toStdString();
                        if (srcname == srcfilename) {
                            QFileInfo destfileInfo(m_data->m_destFileNamePath);
                            QString destfilename(destfileInfo.fileName());
                            gltfdest["images"][i]["uri"] = destfilename.toStdString();
                        }
                    }

                    std::ofstream ofstreamdest(m_data->m_modelDest.toStdString(), std::ios_base::out);
                    if (!ofstreamdest)
                    {
                        QString error = "Error opening gltf compressed version file for update: " + m_data->m_modelDest +"." + strerror(errno);
                        PrintInfo(error.toStdString().c_str());
                        return;
                    }
                    ofstreamdest << gltfdest;
                    ofstreamdest.close();
                }
            }
        }

        // view image
        QVariant v = item->data(TREE_SourceInfo, Qt::UserRole);
        C_Destination_Options *m_data = v.value<C_Destination_Options *>();
        if (m_data)
        {
            // Refresh any changes in current items data
            // like compression time
            if (m_imagePropertyView)
                m_imagePropertyView->OnUpdateData(m_data);

            // Add the image to the diff image list
            if (m_projectview)
            {
                // Add the image to the diff image list if it is not in the list
                if (!(m_projectview->m_ImagesinProjectTrees.contains(m_data->m_destFileNamePath)))
                    m_projectview->m_ImagesinProjectTrees.append(m_data->m_destFileNamePath);
            }

        #ifdef SHOW_DECOMPRESS_IMAGE
            // This can cause issue if image decompresion takes a long time!
            AddImageView(m_data->m_destFileNamePath, item);
        #endif
        }
    }

    // Re-enable relavent tool bar options
    if (compressAct)
        compressAct->setEnabled(true);
    if (imagediffAct)
        imagediffAct->setEnabled(true);
    if (deleteImageAct)
        deleteImageAct->setEnabled(true);

}

void cpMainComponents::onSourceImage(int childCount)
{
    if (MIPGenAct)
        MIPGenAct->setEnabled(true);
    if (deleteImageAct)
        deleteImageAct->setEnabled(true);
    //if (compressAct)
    //    compressAct->setEnabled(childCount > 1);

    if (saveToBatchFileAct)
    {
        if (!saveToBatchFileAct->isEnabled())
            saveToBatchFileAct->setEnabled(childCount > 1);
    }
}

void cpMainComponents::onProjectLoaded(int childCount)
{
    if (saveToBatchFileAct)
    {
        saveToBatchFileAct->setEnabled(childCount > 1);
    }

    if (m_imagePropertyView)
        m_imagePropertyView->OnUpdateData(NULL);
}

void cpMainComponents::onDecompressImage()
{
    if (imagediffAct)
        imagediffAct->setEnabled(true);
    if (MIPGenAct)
        MIPGenAct->setEnabled(false);
    if (deleteImageAct)
        deleteImageAct->setEnabled(true);

    if (saveToBatchFileAct)
    {
        if (!saveToBatchFileAct->isEnabled())
            saveToBatchFileAct->setEnabled(true);
    }

}

void cpMainComponents::OpenCHMFile(QString fileName)
{
    QString str;

    str.append(URL_FILE);
    str.append(qApp->applicationDirPath());
    str.append("/");
    str.append(fileName);
    if (!QDesktopServices::openUrl(QUrl(str)))
    {
        char * ENV;
        str = "";
        ENV = getenv("COMPRESSONATOR_ROOT");
        if (ENV)
        {
            str.append(URL_FILE);
            str.append(ENV);
            str.append("/");
            str.append(fileName);
            QDesktopServices::openUrl(QUrl(str));
        }
    }
}

void cpMainComponents::userGuide()
{
    OpenCHMFile(COMPRESSONATOR_USER_GUIDE);
}

void cpMainComponents::gettingStarted()
{
    OpenCHMFile(COMPRESSONATOR_GETTING_STARTED);
}

void cpMainComponents::onProcessing(QString &FilePathName)
{
    Q_UNUSED(FilePathName);
    // Reserved for future use 
    // to handle any action just before a file is processed 
    // by command line.
}

void cpMainComponents::onAboutToClose(QString &Title)
{
    QList<acCustomDockWidget *> dockWidgets = m_parent->findChildren<acCustomDockWidget *>();
    QListIterator<acCustomDockWidget *> iter(dockWidgets);
    acCustomDockWidget *dock;
    while (iter.hasNext())
    {
        dock = iter.next();
        if (dock)
        {
            QString DockTitle = dock->custTitleBar->getTitle();
            if (DockTitle.compare(Title) == 0)
            {
                delete dock;
                dock = NULL;
                break;
            }
        }
    }
}

