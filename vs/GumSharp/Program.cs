using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading.Tasks;

namespace GumSharp
{
    class Window : IDisposable
    {
        private static bool appInitialized = false;
        internal IntPtr Pointer { get; private set; }
        private Window(IntPtr ptr)
        {
            Pointer = ptr;
        }

        public static Window Open(int width, int height)
        {
            if (!appInitialized) {
                AppSetup_(Process.GetCurrentProcess().Handle);
                appInitialized = true;
            }
            IntPtr ptr = OpenWindow_(width, height);
            return ptr != IntPtr.Zero ? new Window(ptr) : null;
        }

        public void Dispose()
        {
            if (Pointer != IntPtr.Zero) {
                CloseWindow_(Pointer);
            }
            Pointer = IntPtr.Zero;
        }

        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_create_surface")]
        private extern static IntPtr OpenWindow_(int width, int height);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_destroy_surface")]
        private extern static IntPtr CloseWindow_(IntPtr win);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_win32_setup")]
        private extern static IntPtr AppSetup_(IntPtr hInstance);
    }
    class Cell
    {
        internal IntPtr Pointer { get; private set; }
        private Cell(IntPtr ptr)
        {
            Pointer = ptr;
        }

        public static Cell Load(string filename, SkinsPool skins)
        {
            IntPtr skinPtr = skins.Pointer;
            if (skinPtr == IntPtr.Zero)
                return null;
            IntPtr cellPtr = LoadXml_(filename, skinPtr);
            return cellPtr != IntPtr.Zero ? new Cell(cellPtr) : null;
        }


        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_cell_loadxml")]
        private extern static IntPtr LoadXml_(string filename, IntPtr skins);
    }

    class SkinsPool
    {
        internal IntPtr Pointer { get; private set; }
        public SkinsPool()
        {
            Pointer = IntPtr.Zero;
        }

        public void LoadCss(string filename)
        {
            Pointer = LoadCss_(Pointer, filename);
        }
        
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_skins_loadcss")]
        private extern static IntPtr LoadCss_(IntPtr skins, string filename);
    }

    class Manager : IDisposable
    {
        internal IntPtr Pointer { get; private set; }
        private Manager(IntPtr ptr)
        {
            Pointer = ptr;
        }

        public static Manager Create(Cell root, Window win)
        {
            IntPtr ptr = CreateManager_(root.Pointer, win.Pointer);
            return ptr != IntPtr.Zero ? new Manager(ptr) : null;
        }

        public void Dispose()
        {
            if (Pointer != IntPtr.Zero) {
                // CloseManager_(Pointer);
            }
            Pointer = IntPtr.Zero;
        }

        public void Run()
        {
            EventLoop_(Pointer);
        }


        private delegate void InternEventHandler();
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_event_manager")]
        private extern static IntPtr CreateManager_(IntPtr root, IntPtr win);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_event_loop")]
        private extern static void EventLoop_(IntPtr manager);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_refresh")]
        private extern static void Refresh_(IntPtr manager);
        [DllImport("gum", CallingConvention = CallingConvention.Cdecl, EntryPoint = "gum_event_bind")]
        private extern static IntPtr BindEvent_(IntPtr manager, IntPtr cell, int type, InternEventHandler handler);    
    }

    class Program
    {
        static void Main(string[] args)
        {
            SkinsPool skins = new SkinsPool();
            skins.LoadCss("./resx/browser/app2.css");
            Cell root = Cell.Load("./resx/browser/app2.xml", skins);
            using (Window win = Window.Open(680, 425))
            using (Manager ewm = Manager.Create(root, win)) {
                ewm.Run();
            }
        }
    }
}
