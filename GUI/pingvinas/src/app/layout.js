import localFont from "next/font/local";
import "./globals.css";
import Header from "@/components/Header";

const geistSans = localFont({
  src: "./fonts/GeistVF.woff",
  variable: "--font-geist-sans",
  weight: "100 900",
});
const geistMono = localFont({
  src: "./fonts/GeistMonoVF.woff",
  variable: "--font-geist-mono",
  weight: "100 900",
});

export const metadata = {
  title: "Pingvinas",
  description: "",
};

export default function RootLayout({ children }) {
  return (
    <html lang="en">
      <body className="bg-[url('./photos/bg.png')]">
        <Header/>
        {children}
      </body>
    </html>
  );
}
