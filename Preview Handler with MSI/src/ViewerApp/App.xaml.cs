using System.Windows;

namespace ViewerApp
{
    public partial class App : Application
    {
        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

           
            string? filePath = e.Args.Length > 0 ? e.Args[0] : null;

            new MainWindow(filePath).Show();
        }
    }
}
